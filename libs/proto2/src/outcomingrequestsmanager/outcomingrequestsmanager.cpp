#include "outcomingrequestsmanager/outcomingrequestsmanager.h"

#include <utility>

#include "codes/codes.h"
#include "frame/frame.h"

using error_code         = boost::system::error_code;
using InvalidationReason = Proto2::OutcomingRequestContext::AbstractOutcomingRequestContext::InvalidationReason;

namespace Proto2
{

std::shared_ptr<OutcomingRequestsManager>
    OutcomingRequestsManager::create(std::shared_ptr<SocketWrapper> socketWrapper, size_t responseTimeoutMs)
{
    auto self = std::shared_ptr<OutcomingRequestsManager>{ new OutcomingRequestsManager{ std::move(socketWrapper),
                                                                                         responseTimeoutMs } };

    self->establishConnections();

    return self;
}

OutcomingRequestsManager::~OutcomingRequestsManager()
{
    for (auto &[_, pendingRequest] : pendingRequests_)
    {
        pendingRequest->timeoutTimer.cancel();
        pendingRequest->context->invalidate(InvalidationReason::TIMEOUT);
    }
    pendingRequests_.clear();
}

void OutcomingRequestsManager::sendRequest(Request_t request, Context_t context)
{
    boost::asio::post(
        ioContext_,
        [this, request, context, self = shared_from_this()]() mutable
        { sendRequestImpl(std::move(request), std::move(context)); });
}

void OutcomingRequestsManager::sendRequestImpl(Request_t request, Context_t context)
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
            << ", opCode=" << pendingRequest->request->opCode();

    auto frame = Frame::constructRequestHeaderWoOwnership(pendingRequest->requestId, pendingRequest->request->opCode());
    pendingRequest->request->serializePayloadWoOwnership(frame);

    pendingRequest->timeoutTimer.expires_from_now(boost::posix_time::milliseconds{ responseTimeoutMs_ });
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

    socketWrapper_->writeFrame(std::move(frame), [pendingRequest](const error_code &, size_t) {});

    const auto requestId = pendingRequest->requestId;
    pendingRequests_.insert({ requestId, std::move(pendingRequest) });
}

OutcomingRequestsManager::OutcomingRequestsManager(
    std::shared_ptr<SocketWrapper> socketWrapper,
    size_t                         responseTimeoutMs)
    : ioContext_{ socketWrapper->ioContext() },
      socketWrapper_{ std::move(socketWrapper) },
      responseTimeoutMs_{ responseTimeoutMs }
{
}

void OutcomingRequestsManager::establishConnections()
{
    socketWrapper_->incomingFrame.connect(
        [this, weakSelf = weak_from_this()](boost::asio::const_buffer frame)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;
            else
                onIncomingFrame(frame);
        });
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
        EN_LOGW << "Recived response for request " << requestId << ", but it is not marked as pending";
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

}    // namespace Proto2
