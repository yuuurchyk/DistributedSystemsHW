#pragma once

#include "outcomingrequestsmanager.h"

#include "frame.h"

namespace Proto
{

template <Concepts::Request Request>
bool OutcomingRequestsManager::PendingRequest<Request>::readResponseBody(
    Reflection::DeserializationContext<boost::asio::const_buffer> &context)
{
    auto optResponse = context.deserialize<Concepts::Response_t<Request>>();

    if (optResponse.has_value() && context.atEnd())
    {
        promise.set_value(std::move(optResponse.value()));
        return true;
    }
    else
    {
        return false;
    }
}

template <Concepts::Request Request>
void OutcomingRequestsManager::PendingRequest<Request>::invalidateBadFrame()
{
    try
    {
        throw BadFrameException{};
    }
    catch (...)
    {
        promise.set_exception(std::current_exception());
    }
}

template <Concepts::Request Request>
void OutcomingRequestsManager::PendingRequest<Request>::invalidateDisconnected()
{
    try
    {
        throw DisconnectedException{};
    }
    catch (...)
    {
        promise.set_exception(std::current_exception());
    }
}

template <Concepts::Request Request>
void OutcomingRequestsManager::PendingRequest<Request>::invalidateTimeout()
{
    try
    {
        throw TimeoutException{};
    }
    catch (...)
    {
        promise.set_exception(std::current_exception());
    }
}

template <Concepts::Request Request>
boost::future<typename Concepts::Response_t<Request>>
    OutcomingRequestsManager::send(std::shared_ptr<const Request> request)
{
    auto pendingRequest = std::make_shared<PendingRequest<Request>>();

    boost::asio::post(
        socketWrapper_->ioContext(),
        [this, request, pendingRequest, self = shared_from_this()]()
        {
            const auto requestId = getNextRequestId();

            {
                auto context = std::make_shared<Reflection::SerializationContext>();

                auto requestHeader = std::make_shared<RequestHeader>(requestId, Concepts::OpCode_v<Request>);

                context->serializeAndHold(requestHeader);
                context->serializeAndHold(request);

                socketWrapper_->send(std::move(context));
            }

            auto timeoutTimer = std::make_shared<boost::asio::deadline_timer>(
                socketWrapper_->ioContext(), boost::posix_time::milliseconds{ sendTimeoutMs_ });

            timeoutTimer->async_wait(
                [this, requestId, weakSelf = weak_from_this()](const boost::system::error_code &ec)
                {
                    auto self = weakSelf.lock();
                    if (ec || self == nullptr)
                        return;

                    EN_LOGW << "Timeout occured for request: " << requestId;

                    {
                        const auto it = pendingRequests_.find(requestId);
                        if (it != pendingRequests_.end())
                        {
                            it->second->invalidateTimeout();
                            pendingRequests_.erase(it);
                        }
                    }
                    {
                        const auto it = pendingRequestsTimers_.find(requestId);
                        if (it != pendingRequestsTimers_.end())
                            pendingRequestsTimers_.erase(it);
                    }
                });

            pendingRequests_.insert({ requestId, std::move(pendingRequest) });
            pendingRequestsTimers_.insert({ requestId, timeoutTimer });
        });

    return pendingRequest->promise.get_future();
}

}    // namespace Proto
