#include "outcomingrequestsmanager/outcomingrequestsmanager.h"

#include <utility>

#include "net-utils/launchwithdelay.h"

#include "codes/codes.h"
#include "frame/frame.h"

using error_code         = boost::system::error_code;
using InvalidationReason = Proto::OutcomingRequestContext::InvalidationReason;

namespace Proto
{
std::shared_ptr<OutcomingRequestsManager> OutcomingRequestsManager::create(
    std::string                    id,
    std::shared_ptr<SocketWrapper> socketWrapper,
    Utils::duration_milliseconds_t responseTimeout)
{
    auto self = std::shared_ptr<OutcomingRequestsManager>{ new OutcomingRequestsManager{
        std::move(id), std::move(socketWrapper), responseTimeout } };

    self->establishConnections();

    return self;
}

OutcomingRequestsManager::~OutcomingRequestsManager()
{
    invalidateAllPendingRequests(InvalidationReason::PEER_DISCONNECTED);
}

void OutcomingRequestsManager::sendRequest(
    Request_t                      request,
    Context_t                      context,
    Utils::duration_milliseconds_t artificialDelay)
{
    boost::asio::post(
        ioContext_,
        [this,
         request = std::move(request),
         context = std::move(context),
         artificialDelay,
         weakSelf = weak_from_this()]() mutable
        {
            const auto self = weakSelf.lock();

            if (self == nullptr)
                context->invalidate(InvalidationReason::PEER_DISCONNECTED);
            else
                sendRequestImpl(std::move(request), std::move(context), artificialDelay);
        });
}

void OutcomingRequestsManager::sendRequestImpl(
    Request_t                      request,
    Context_t                      context,
    Utils::duration_milliseconds_t artificialDelay)
{
    auto pendingRequest =
        PendingRequest::create(ioContext_, requestIdCounter_++, std::move(request), std::move(context));

    if (const auto it = pendingRequests_.find(pendingRequest->requestId); it != pendingRequests_.end())
    {
        EN_LOGW << "Request id wrap around, should never happen";
        pendingRequest->context->invalidate(InvalidationReason::TIMEOUT);
        return;
    }

    EN_LOGD << "sending request through SocketWrapper: requestId = " << pendingRequest->requestId
            << ", opCode=" << pendingRequest->request->opCode() << ", delayed by " << artificialDelay.count() << "ms";

    auto frame = Frame::constructRequestHeaderWoOwnership(pendingRequest->requestId, pendingRequest->request->opCode());
    pendingRequest->request->serializePayloadWoOwnership(frame);

    pendingRequest->timeoutTimer.expires_from_now(Utils::toPosixTime(responseTimeout_));
    pendingRequest->timeoutTimer.async_wait(
        [this, requestId = pendingRequest->requestId, weakSelf = weak_from_this()](const error_code &ec)
        {
            if (ec)
                return;

            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            invalidatePendingRequest(requestId, InvalidationReason::TIMEOUT);
        });

    const auto requestId = pendingRequest->requestId;
    pendingRequests_.insert({ requestId, pendingRequest });

    NetUtils::launchWithDelay(
        ioContext_,
        Utils::toPosixTime(artificialDelay),
        [this,
         frame = std::move(frame),
         requestId,
         pendingRequest = std::move(pendingRequest),
         weakSelf       = weak_from_this()]() mutable
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (socketWrapper_->wasInvalidated())
            {
                invalidatePendingRequest(requestId, InvalidationReason::PEER_DISCONNECTED);
            }
            else
            {
                socketWrapper_->writeFrame(
                    std::move(frame), [pendingRequest = std::move(pendingRequest)](const error_code &, size_t) {});
            }
        });
}

OutcomingRequestsManager::OutcomingRequestsManager(
    std::string                    id,
    std::shared_ptr<SocketWrapper> socketWrapper,
    Utils::duration_milliseconds_t responseTimeout)
    : logger::StringIdEntity<OutcomingRequestsManager>{ std::move(id) },
      ioContext_{ socketWrapper->executionContext() },
      socketWrapper_{ std::move(socketWrapper) },
      responseTimeout_{ responseTimeout }
{
}

void OutcomingRequestsManager::establishConnections()
{
    socketWrapper_->incomingFrame.connect(Utils::slot_type<boost::asio::const_buffer>{
        &OutcomingRequestsManager::onIncomingFrame, this, boost::placeholders::_1 }
                                              .track_foreign(weak_from_this()));

    socketWrapper_->invalidated.connect(Utils::slot_type<>{
        &OutcomingRequestsManager::invalidateAllPendingRequests, this, InvalidationReason::PEER_DISCONNECTED }
                                            .track_foreign(weak_from_this()));
}

void OutcomingRequestsManager::onResponseRecieved(size_t requestId, boost::asio::const_buffer payload)
{
    EN_LOGD << "onResponseRecieved(requestId=" << requestId << ")";

    const auto it = pendingRequests_.find(requestId);
    if (it == pendingRequests_.end())
    {
        EN_LOGW << "Recieved response for request " << requestId << ", but it is not marked as pending";
        return;
    }

    auto &pendingRequest = it->second;
    pendingRequest->timeoutTimer.cancel();
    pendingRequest->context->onResponseRecieved(payload);
    pendingRequests_.erase(it);
}

void OutcomingRequestsManager::onIncomingFrame(boost::asio::const_buffer frame)
{
    const auto optEventType = Frame::parseEventType(frame);
    if (!optEventType.has_value() || optEventType.value() != EventType::RESPONSE)
        return;

    const auto optResponseFrame = Frame::parseResponseFrame(frame);
    if (!optResponseFrame.has_value())
    {
        EN_LOGW << "Failed to parse response frame";
        return;
    }

    auto responseFrame = std::move(optResponseFrame.value());

    EN_LOGD << "processing response frame for request with id " << responseFrame.requestId;

    onResponseRecieved(responseFrame.requestId, responseFrame.payload);
}

void OutcomingRequestsManager::invalidatePendingRequest(size_t requestId, InvalidationReason reason)
{
    EN_LOGW << "invalidatePendingRequest(requestId=" << requestId << ", reason=" << reason << ")";

    const auto it = pendingRequests_.find(requestId);
    if (it == pendingRequests_.end())
        return;

    auto &pendingRequest = it->second;
    pendingRequest->timeoutTimer.cancel();
    pendingRequest->context->invalidate(reason);

    pendingRequests_.erase(it);
}

void OutcomingRequestsManager::invalidateAllPendingRequests(InvalidationReason reason)
{
    for (auto &[requestId, pendingRequest] : pendingRequests_)
    {
        EN_LOGW << "invalidating request with id " << requestId << ", reason=" << reason;
        pendingRequest->timeoutTimer.cancel();
        pendingRequest->context->invalidate(reason);
    }

    pendingRequests_.clear();
}

auto OutcomingRequestsManager::PendingRequest::create(
    boost::asio::io_context &ioContext,
    size_t                   requestId,
    Request_t                request,
    Context_t                context) -> std::shared_ptr<PendingRequest>
{
    return std::shared_ptr<PendingRequest>{ new PendingRequest{
        ioContext, requestId, std::move(request), std::move(context) } };
}

OutcomingRequestsManager::PendingRequest::PendingRequest(
    boost::asio::io_context &ioContext,
    size_t                   requestId,
    Request_t                request,
    Context_t                context)
    : requestId{ requestId }, timeoutTimer{ ioContext }, request{ std::move(request) }, context{ std::move(context) }
{
}

}    // namespace Proto
