#include "outcomingrequestsmanager.h"

#include <algorithm>
#include <utility>

#include "reflection/deserialization.h"

#include "frame.h"

namespace Proto
{
std::shared_ptr<OutcomingRequestsManager>
    OutcomingRequestsManager::create(std::shared_ptr<SocketWrapper> socketWrapper, size_t sendTimeoutMs)
{
    return std::shared_ptr<OutcomingRequestsManager>{ new OutcomingRequestsManager{ std::move(socketWrapper),
                                                                                    sendTimeoutMs } };
}

void OutcomingRequestsManager::registerConnections()
{
    incomingBufferConnection_ = socketWrapper_->incomingBuffer.connect(
        [this, weakSelf = weak_from_this()](boost::asio::const_buffer buffer)
        {
            auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            onIncomingBuffer(buffer);
        });
    invalidatedConnection_ = socketWrapper_->invalidated.connect(
        [this, weakSelf = weak_from_this()]()
        {
            auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            onInvalidated();
        });
}

OutcomingRequestsManager::~OutcomingRequestsManager()
{
    incomingBufferConnection_.disconnect();
    invalidatedConnection_.disconnect();
    onInvalidated();
}

OutcomingRequestsManager::OutcomingRequestsManager(std::shared_ptr<SocketWrapper> socketWrapper, size_t sendTimeoutMs)
    : socketWrapper_{ std::move(socketWrapper) }, sendTimeoutMs_{ std::max<size_t>(sendTimeoutMs, 1) }
{
}

size_t OutcomingRequestsManager::getNextRequestId()
{
    return requestIdCounter_.fetch_add(1, std::memory_order_relaxed);
}

void OutcomingRequestsManager::onInvalidated()
{
    for (auto &[_, timer] : pendingRequestsTimers_)
        timer->cancel();

    for (auto &[_, pendingRequest] : pendingRequests_)
        pendingRequest->invalidateDisconnected();

    pendingRequestsTimers_.clear();
    pendingRequests_.clear();
}

void OutcomingRequestsManager::onIncomingBuffer(boost::asio::const_buffer buffer)
{
    auto context = Reflection::DeserializationContext{ buffer };

    const auto optHeader = context.deserialize<IncomingHeader>();
    if (!optHeader.has_value())
    {
        EN_LOGW << "Failed to read incoming header";
        return;
    }

    const auto &header = optHeader.value();

    if (header.eventType != EventType::RESPONSE)
        return;

    const auto requestId = header.requestId;

    if (const auto it = pendingRequests_.find(requestId); it != pendingRequests_.end())
    {
        auto &pendingRequest = *it->second;

        if (!pendingRequest.readResponseBody(context))
            pendingRequest.invalidateBadFrame();

        pendingRequests_.erase(it);
    }

    if (const auto it = pendingRequestsTimers_.find(requestId); it != pendingRequestsTimers_.end())
    {
        auto &timer = *it->second;
        timer.cancel();

        pendingRequestsTimers_.erase(it);
    }
}

}    // namespace Proto
