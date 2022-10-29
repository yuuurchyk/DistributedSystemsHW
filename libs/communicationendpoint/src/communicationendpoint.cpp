#include "communicationendpoint/communicationendpoint.h"

#include <utility>

#include <boost/system/error_code.hpp>

#include "logger/logger.h"
#include "protocol/buffer.h"

using namespace boost::asio;
using namespace protocol;
using error_code = boost::system::error_code;

std::shared_ptr<CommunicationEndpoint>
    CommunicationEndpoint::create(io_context         &context,
                                  ip::tcp::socket     socket,
                                  request_callback_fn requestCallback)
{
    return std::shared_ptr<CommunicationEndpoint>{ new CommunicationEndpoint{
        context, std::move(socket), std::move(requestCallback) } };
}

void CommunicationEndpoint::run()
{
    readFrameSize();
}

size_t CommunicationEndpoint::getNextRequestId()
{
    return requestIdCounter_.fetch_add(1, std::memory_order_relaxed);
}

void CommunicationEndpoint::sendRequest(request::Request     arg_request,
                                        response_callback_fn arg_responseCallback,
                                        const boost::posix_time::milliseconds &timeout)
{
    if (!arg_request.valid() ||
        pendingRequests_.find(arg_request.requestId()) != pendingRequests_.end())
    {
        ID_LOGE << "Invalid request, executing response callback right away";
        if (arg_responseCallback != nullptr)
            arg_responseCallback(/* response */ {}, std::move(arg_request));
        return;
    }

    ID_LOGI << "Sending request: " << arg_request;

    const auto id             = arg_request.requestId();
    auto       pendingRequest = PendingRequest::create(
        context_, std::move(arg_request), std::move(arg_responseCallback));

    // NOTE: don't use arg_* variables from now on

    pendingRequests_.insert({ id, pendingRequest });

    async_write(
        socket_,
        buffer(pendingRequest->request.buffer().data(),
               pendingRequest->request.buffer().size()),
        [this, self = shared_from_this(), pendingRequest](const error_code &error, size_t)
        {
            if (error)
            {
                ID_LOGE << "Failed to write request frame";
            }
        });

    pendingRequest->timeoutTimer.expires_from_now(timeout);
    pendingRequest->timeoutTimer.async_wait(
        [this, self = shared_from_this(), pendingRequest = std::move(pendingRequest), id](
            const error_code &error)
        {
            if (!error)
                return;

            ID_LOGW << "timeout expired for request with id=" << id;

            if (pendingRequest->responseCallback != nullptr)
            {
                pendingRequest->responseCallback(/* response */ {},
                                                 std::move(pendingRequest->request));
            }

            if (auto it = pendingRequests_.find(id); it != pendingRequests_.end())
                pendingRequests_.erase(it);
        });
}

void CommunicationEndpoint::sendResponse(response::Response response)
{
    ID_LOGI << "sending response: " << response;

    if (!response.valid())
    {
        ID_LOGW << "response is not valid, exiting";
        return;
    }

    auto  frameOwner = std::make_unique<Frame>(std::move(response));
    auto &frame      = *frameOwner;

    async_write(socket_,
                buffer(frame.buffer().data(), frame.buffer().size()),
                [this, self = shared_from_this(), frameOwner = std::move(frameOwner)](
                    const error_code &error, size_t)
                {
                    if (error)
                    {
                        ID_LOGE << "Failed to write response frame";
                    }
                });
}

CommunicationEndpoint::CommunicationEndpoint(io_context         &context,
                                             ip::tcp::socket     socket,
                                             request_callback_fn requestCallback)
    : context_{ context },
      socket_{ std::move(socket) },
      requestCallback_{ std::move(requestCallback_) }
{
}

void CommunicationEndpoint::readFrameSize()
{
    ID_LOGI << "Reading frame size";
    async_read(socket_,
               buffer(&frameSize_, sizeof(size_t)),
               transfer_exactly(sizeof(size_t)),
               [this, self = shared_from_this()](const error_code &error, size_t)
               {
                   if (error)
                   {
                       ID_LOGE << "Failed to read from socket: " << error.message();
                       return;
                   }

                   ID_LOGI << "Frame size read: " << frameSize_;
                   readFrame();
               });
}

void CommunicationEndpoint::readFrame()
{
    if (frameSize_ < sizeof(size_t))
    {
        ID_LOGE << "invalid frame size, exiting";
        return;
    }

    auto  bufOwner = std::make_unique<Buffer>(frameSize_);
    auto &buf = *bufOwner;    // note: this is needed since arguments evaluation order in
                              // async_read() is undefined

    if (buf.invalidated())
    {
        ID_LOGE << "Failed to allocate buffer";
        return;
    }

    *reinterpret_cast<size_t *>(buf.data()) = frameSize_;

    async_read(socket_,
               buffer(buf.data() + sizeof(size_t), frameSize_ - sizeof(size_t)),
               transfer_exactly(frameSize_ - sizeof(size_t)),
               [this, self = shared_from_this(), bufOwner = std::move(bufOwner)](
                   const error_code &error, size_t)
               {
                   if (error)
                   {
                       ID_LOGE << "Failed to read frame";
                   }
                   else
                   {
                       ID_LOGI << "Read into buffer successfully";

                       bufOwner->setSize(bufOwner->capacity());
                       parseBuffer(std::move(*bufOwner));

                       readFrameSize();
                   }
               });
}

void CommunicationEndpoint::parseBuffer(Buffer buffer)
{
    ID_LOGI << "parsing buffer, creating frame";

    auto frame = Frame{ std::move(buffer) };
    if (!frame.valid())
    {
        ID_LOGW << "invalid frame, exiting";
        return;
    }
    else
    {
        ID_LOGI << "incoming frame: " << frame;
    }

    switch (frame.event())
    {
    case codes::Event::REQUEST:
    {
        if (requestCallback_ != nullptr)
            requestCallback_(request::Request{ std::move(frame) });
        break;
    }
    case codes::Event::RESPONSE:
    {
        parseResponse(response::Response{ std::move(frame) });
        break;
    }
    }
}

void CommunicationEndpoint::parseResponse(response::Response response)
{
    ID_LOGI << "parsing response for " << response;

    if (!response.valid())
    {
        ID_LOGW << "invalid response, exiting";
        return;
    }

    auto it = pendingRequests_.find(response.requestId());

    if (it == pendingRequests_.end())
    {
        ID_LOGI << "Could not find pending request for response, exiting";
        return;
    }

    auto pendingRequest = std::move(it->second);
    pendingRequests_.erase(it);

    ID_LOGI << "Found pending request";
    ID_LOGI << "pending request: " << pendingRequest->request;
    ID_LOGI << "response       : " << response;

    if (pendingRequest->responseCallback != nullptr)
    {
        pendingRequest->responseCallback(std::move(response),
                                         std::move(pendingRequest->request));
    }

    pendingRequest->timeoutTimer.cancel();
}

auto CommunicationEndpoint::PendingRequest::create(io_context          &context,
                                                   request::Request     request,
                                                   response_callback_fn responseCallback)
    -> std::shared_ptr<PendingRequest>
{
    return std::shared_ptr<PendingRequest>{ new PendingRequest{
        context, std::move(request), responseCallback } };
}

CommunicationEndpoint::PendingRequest::PendingRequest(
    io_context          &context,
    request::Request     request,
    response_callback_fn responseCallback)
    : request{ std::move(request) },
      responseCallback{ std::move(responseCallback) },
      timeoutTimer{ context }
{
}
