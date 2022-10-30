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
 * @note CommunicationEndpoint resides in a single thread, so the writes
 * from different threads are not happening concurrently
 */
class CommunicationEndpoint : public std::enable_shared_from_this<CommunicationEndpoint>,
                              private logger::IdEntity<CommunicationEndpoint>
{
    DISABLE_COPY_MOVE(CommunicationEndpoint);

public:
    // when the endpoint recieved a request from the other side
    using request_callback_fn = std::function<void(std::shared_ptr<CommunicationEndpoint>,
                                                   protocol::request::Request request)>;
    // when the net sent a response to the endpoint's request, added via sendRequest()
    using response_callback_fn =
        std::function<void(std::shared_ptr<CommunicationEndpoint>,
                           std::optional<protocol::response::Response> response,
                           protocol::request::Request                  request)>;
    using invalidation_callback_fn =
        std::function<void(std::shared_ptr<CommunicationEndpoint>)>;

    static std::shared_ptr<CommunicationEndpoint> create(boost::asio::io_context &,
                                                         boost::asio::ip::tcp::socket,
                                                         request_callback_fn,
                                                         invalidation_callback_fn);

    void run();

    size_t getNextRequestId();
    void   sendRequest(protocol::request::Request             request,
                       response_callback_fn                   responseCallback,
                       const boost::posix_time::milliseconds &timeout =
                           boost::posix_time::milliseconds{ 1500 });
    void   sendResponse(protocol::response::Response response);

private:
    CommunicationEndpoint(boost::asio::io_context &,
                          boost::asio::ip::tcp::socket,
                          request_callback_fn,
                          invalidation_callback_fn);

    void readFrameSize();
    void readFrame();
    void parseBuffer(protocol::Buffer);
    void parseResponse(protocol::response::Response);

    void invalidate();

    boost::asio::io_context       &context_;
    boost::asio::ip::tcp::socket   socket_;
    const request_callback_fn      requestCallback_;
    const invalidation_callback_fn invalidationCallback_;

    size_t              frameSize_{};
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
