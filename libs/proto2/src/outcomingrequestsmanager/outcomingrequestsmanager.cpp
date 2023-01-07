#include "outcomingrequestsmanager/outcomingrequestsmanager.h"

#include <stdexcept>
#include <utility>

#include "codes/codes.h"
#include "frame/frame.hpp"

using error_code         = boost::system::error_code;
using InvalidationReason = Proto2::OutcomingRequestContext::AbstractOutcomingRequestContext::InvalidationReason;

namespace Proto2
{
OutcomingRequestsManager::Pending::Pending(
    boost::asio::io_context                                                  &ioContext,
    size_t                                                                    id,
    std::shared_ptr<Request::AbstractRequest>                                 request,
    std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext> context)
    : id{ id }, timeoutTimer{ ioContext }, request{ std::move(request) }, context{ std::move(context) }
{
}

std::shared_ptr<OutcomingRequestsManager>
    OutcomingRequestsManager::create(std::shared_ptr<SocketWrapper> socketWrapper, size_t responseTimeoutMs)
{
    if (socketWrapper == nullptr)
    {
        LOGE << "socket wrapper cannot be nullptr";
        throw std::logic_error{ "socket wrapper cannot be nullptr" };
    }

    auto &ioContext = socketWrapper->ioContext();

    return std::shared_ptr<OutcomingRequestsManager>{ new OutcomingRequestsManager{
        ioContext, std::move(socketWrapper), responseTimeoutMs } };
}

void OutcomingRequestsManager::send(
    std::shared_ptr<Request::AbstractRequest>                                 request,
    std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext> context)
{
    if (request == nullptr)
    {
        EN_LOGW << "request should not be nullptr, not sending";
        return;
    }
    if (context == nullptr)
    {
        EN_LOGW << "request context should not be nullptr, not sending";
        return;
    }

    auto pending = Pending{ ioContext_, getNextRequestId(), std::move(request), std::move(context) };

    boost::asio::post(
        ioContext_,
        [this, pending = std::move(pending), self = shared_from_this()]() mutable { sendImpl(std::move(pending)); });
}

OutcomingRequestsManager::OutcomingRequestsManager(
    boost::asio::io_context       &ioContext,
    std::shared_ptr<SocketWrapper> socketWrapper,
    size_t                         responseTimeoutMs)
    : ioContext_{ ioContext }, socketWrapper_{ std::move(socketWrapper) }, responseTimeoutMs_{ responseTimeoutMs }
{
    if (socketWrapper_ == nullptr)
    {
        EN_LOGE << "socket wrapper cannot be nullptr";
        throw std::logic_error{ "socket wrapper cannot be nullptr" };
    }

    socketWrapper_->incomingFrame.connect(
        [this, weakSelf = weak_from_this()](boost::asio::const_buffer frame)
        {
            auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            onIncomingFrame(frame);
        });
}

size_t OutcomingRequestsManager::getNextRequestId()
{
    return requestIdCounter_.fetch_add(1, std::memory_order_relaxed);
}

void OutcomingRequestsManager::sendImpl(Pending pending)
{
    const auto id = pending.id;

    pending.timeoutTimer.expires_from_now(boost::posix_time::milliseconds{ responseTimeoutMs_ });
    pending.timeoutTimer.async_wait(
        [this, id, self = shared_from_this()](const error_code &ec)
        {
            if (ec)
                return;
            else
                onExpired(id);
        });

    if (requests_.find(id) != requests_.end())
    {
        EN_LOGE << "Request id wrap around, should never happen";
        pending.context->invalidate(InvalidationReason::TIMEOUT);
        return;
    }

    EN_LOGI << "sending request: id=" << pending.id << ", op code=" << pending.request->opCode();

    requestIdBuffer_  = id;
    auto requestFrame = Frame::constructRequestHeaderWoOwnership(requestIdBuffer_, pending.request->opCode());
    pending.request->serializePayload(requestFrame);

    socketWrapper_->writeFrame(
        std::move(requestFrame),
        [this, id, request = pending.request, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (!ec)
                return;

            if (const auto it = requests_.find(id); it != requests_.end())
            {
                it->second.context->invalidate(InvalidationReason::DISCONNECTED);
                requests_.erase(it);
            }
        });

    requests_.insert({ id, std::move(pending) });
}

void OutcomingRequestsManager::onExpired(size_t id)
{
    const auto it = requests_.find(id);

    if (it == requests_.end())
        return;

    auto &pending = it->second;
    pending.context->invalidate(InvalidationReason::TIMEOUT);

    requests_.erase(it);
}

void OutcomingRequestsManager::onResponseRecieved(size_t id, boost::asio::const_buffer payload)
{
    const auto it = requests_.find(id);

    if (it == requests_.end())
        return;

    auto &pending = it->second;
    pending.context->onResponseRecieved(payload);

    requests_.erase(it);
}

void OutcomingRequestsManager::onInvalidated()
{
    for (auto &[_, pending] : requests_)
        pending.context->invalidate(InvalidationReason::DISCONNECTED);
    requests_.clear();
}

void OutcomingRequestsManager::onIncomingFrame(boost::asio::const_buffer frame)
{
    const auto optEventType = Frame::parseEventType(frame);
    if (!optEventType.has_value() || optEventType.value() != EventType::RESPONSE)
        return;

    const auto optResponseFrame = Frame::parseResponseFrame(frame);
    if (!optResponseFrame.has_value())
    {
        EN_LOGW << "Recieved invalid response frame, invalidating socket";
        return socketWrapper_->invalidate();
    }

    const auto &responseFrame = optResponseFrame.value();
    EN_LOGI << "Recieved response for request id = " << responseFrame.requestId;

    onResponseRecieved(responseFrame.requestId, responseFrame.payload);
}

}    // namespace Proto2
