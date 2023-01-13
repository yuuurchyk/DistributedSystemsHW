#include "outcomingrequestsmanager/outcomingrequestsmanager.h"

#include <utility>

#include "net-utils/launchwithdelay.h"

#include "codes/codes.h"
#include "frame/frame.h"

using error_code         = boost::system::error_code;
using InvalidationReason = Proto::OutcomingRequestContext::AbstractOutcomingRequestContext::InvalidationReason;

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
    invalidateAll();
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
         self = shared_from_this()]() mutable
        { sendRequestImpl(std::move(request), std::move(context), artificialDelay); });
}

void OutcomingRequestsManager::sendRequestImpl(
    Request_t                      request,
    Context_t                      context,
    Utils::duration_milliseconds_t artificialDelay)
{
    auto pendingRequest =
        PendingRequest::create(ioContext_, ++requestIdCounter_, std::move(request), std::move(context));

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

            onExpired(requestId);
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
                onPeerDisconnected(requestId);
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
    incomingFrameConnection_ =
        socketWrapper_->incomingFrame.connect([this](boost::asio::const_buffer frame) { onIncomingFrame(frame); });
    invalidatedConnection_ = socketWrapper_->invalidated.connect([this]() { invalidateAll(); });
}

void OutcomingRequestsManager::onExpired(size_t requestId)
{
    EN_LOGD << "onExpired(requestId=" << requestId << ")";

    const auto it = pendingRequests_.find(requestId);
    if (it == pendingRequests_.end())
        return;

    auto &pendingRequest = it->second;
    pendingRequest->context->invalidate(InvalidationReason::TIMEOUT);
    pendingRequests_.erase(it);
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

void OutcomingRequestsManager::onPeerDisconnected(size_t requestId)
{
    const auto it = pendingRequests_.find(requestId);
    if (it == pendingRequests_.end())
        return;

    EN_LOGW << "onPeerDisconnected(requestId=" << requestId << ")";

    auto &pendingRequest = it->second;
    pendingRequest->timeoutTimer.cancel();
    pendingRequest->context->invalidate(InvalidationReason::PEER_DISCONNECTED);
}

void OutcomingRequestsManager::invalidateAll()
{
    for (auto &[requestId, pendingRequest] : pendingRequests_)
    {
        EN_LOGW << "peer disconnected for request " << requestId;

        pendingRequest->timeoutTimer.cancel();
        pendingRequest->context->invalidate(InvalidationReason::PEER_DISCONNECTED);
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
