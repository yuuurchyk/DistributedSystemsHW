#include "communicationendpoint/communicationendpoint.h"

#include <utility>

#include <boost/system/error_code.hpp>

#include "logger/logger.h"

using namespace boost::asio;
using namespace protocol;
using error_code = boost::system::error_code;

std::shared_ptr<CommunicationEndpoint>
    CommunicationEndpoint::create(io_context                           &context,
                                  ip::tcp::socket                       socket,
                                  std::function<void(request::Request)> requestCallback)
{
    return std::shared_ptr<CommunicationEndpoint>{ new CommunicationEndpoint{
        context, std::move(socket), std::move(requestCallback) } };
}

void CommunicationEndpoint::start()
{
    // TODO: implement
}

void CommunicationEndpoint::sendRequest(request::Request     request,
                                        response_callback_fn responseCallback,
                                        const boost::posix_time::milliseconds &timeout)
{
    if (!request.valid())
    {
        LOGI << "request" << request.requestId()
             << " is not valid, creating response callback right away without sending";
        if (responseCallback != nullptr)
            responseCallback(/* response */ {}, std::move(request));
        return;
    }
    else
    {
                return sendValidRequestImpl(
            std::make_shared<request::Request>(std::move(request)),
            std::make_shared<response_callback_fn>(std::move(responseCallback)),
            timeout);
    }
}

void CommunicationEndpoint::sendValidRequestImpl(
    std::shared_ptr<request::Request>      request,
    std::shared_ptr<response_callback_fn>  responseCallback,
    const boost::posix_time::milliseconds &timeout)
{
    auto timeoutTimer = std::make_shared<deadline_timer>(context_);
    timeoutTimer->expires_from_now(timeout);
    timeoutTimer->async_wait(
        [timeoutTimer, endpoint = shared_from_this(), request](const error_code &error)
        {
            if (error)
                return;

            LOGI << "timer expired for request "
        });

    const auto requestId = request.requestId();

    auto it = pendingRequests_.insert_or_assign(
        requestId,
        PendingRequest::create(*this, timeout, requestPtr, std::move(responseCallback)));

    const auto requestBuffer = requestPtr->buffer();
    async_write(socket_,
                buffer(requestBuffer.data(), requestBuffer.size()),
                [requestPtr](const error_code &error)
                {
                    if (error)
                        LOGI << "failed to write request " << requestPtr->requestId()
                             << " through network";
                    else
                        LOGI << "successfully wrote request " << requestPtr->requestId()
                             << " though network";
                });
}

CommunicationEndpoint::CommunicationEndpoint(boost::asio::io_context     &context,
                                             boost::asio::ip::tcp::socket socket,
                                             request_callback_fn          requestCallback)
    : context_{ context },
      socket_{ std::move(socket) },
      requestCallback_{ std::move(requestCallback) }
{
}

CommunicationEndpoint::PendingRequest::PendingRequest(
    CommunicationEndpoint                 &endpoint,
    const boost::posix_time::milliseconds &timeout,
    std::shared_ptr<request::Request>      request,
    response_callback_fn                   responseCallback)
    : request{ std::move(request) },
      responseCallback{ std::move(responseCallback) },
      timer{ endpoint.context_ }
{
    init(endpoint, timeout);
}

void CommunicationEndpoint::PendingRequest::init(
    CommunicationEndpoint                 &endpoint,
    const boost::posix_time::milliseconds &timeout)
{
    if (!request->valid())
        return;
    LOGI << "request is valid, sending through";

    timer.expires_from_now(timeout);
    timer.async_wait(
        [endpoint  = endpoint.shared_from_this(),
         requestId = request->requestId(),
         request   = this->request](const error_code &error) mutable
        {
            if (error)
                return;
            LOGI << "timer expired for request " << requestId;

            const auto it = endpoint->pendingRequests_.find(requestId);

            if (it == endpoint->pendingRequests_.end())
            {
                LOGI << "could not find request in pending, exiting";
                return;
            }
            else
            {
                auto responseCallback = std::move(it->second.responseCallback);
                auto request          = std::move(it->second.request);

                endpoint->pendingRequests_.erase(it);
                endpoint.reset();

                responseCallback(/* response */ {}, std::move(request));
            }
        });
}
