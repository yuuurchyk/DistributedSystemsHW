#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "protocol/buffer.h"
#include "protocol/request/request.h"
#include "protocol/response/response.h"

/**
 * @brief Represents a tcp communication channel for custom protocol.
 *
 * @note getNextRequestId() function is thread safe
 * @note sendRequest() function is thread safe
 * @note it is guaranteed that response_callback_fn will be called
 * (either after timeout or when response arrives)
 *
 * CommunicationEndpoint is meant to have its own io_context (i.e. reside in a
 * separate thread)
 */
class CommunicationEndpoint : public std::enable_shared_from_this<CommunicationEndpoint>,
                              private logger::IdEntity<CommunicationEndpoint>
{
    DISABLE_COPY_MOVE(CommunicationEndpoint);

public:
    // when the endpoint recieved a request from the other side
    using request_callback_fn = std::function<void(protocol::request::Request request)>;
    // when the net sent a response to the endpoint's request, added via sendRequest()
    using response_callback_fn =
        std::function<void(std::optional<protocol::response::Response> response,
                           protocol::request::Request                  request)>;

    static std::shared_ptr<CommunicationEndpoint>
        create(boost::asio::io_context     &context,
               boost::asio::ip::tcp::socket socket,
               request_callback_fn          requestCallback);

    void run();

    size_t getNextRequestId();
    void   sendRequest(protocol::request::Request             request,
                       response_callback_fn                   responseCallback,
                       const boost::posix_time::milliseconds &timeout =
                           boost::posix_time::milliseconds{ 1500 });
    void   sendResponse(protocol::response::Response response);

private:
    CommunicationEndpoint(boost::asio::io_context     &context,
                          boost::asio::ip::tcp::socket socket,
                          request_callback_fn          requestCallback);

    void readFrameSize();
    void readFrame();
    void parseBuffer(protocol::Buffer);
    void parseResponse(protocol::response::Response);

    boost::asio::io_context     &context_;
    boost::asio::ip::tcp::socket socket_;
    size_t                       frameSize_{};
    const request_callback_fn    requestCallback_;

    std::atomic<size_t> requestIdCounter_{};

    struct PendingRequest : public std::enable_shared_from_this<PendingRequest>
    {
        DISABLE_COPY_MOVE(PendingRequest);

        static std::shared_ptr<PendingRequest> create(boost::asio::io_context &,
                                                      protocol::request::Request,
                                                      response_callback_fn);

        protocol::request::Request  request;
        response_callback_fn        responseCallback;
        boost::asio::deadline_timer timeoutTimer;

    private:
        PendingRequest(boost::asio::io_context &,
                       protocol::request::Request,
                       response_callback_fn);
    };

    std::unordered_map<size_t, std::shared_ptr<PendingRequest>> pendingRequests_;
};
