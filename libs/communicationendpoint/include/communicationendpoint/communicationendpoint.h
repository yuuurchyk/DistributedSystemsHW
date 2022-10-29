#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include <boost/asio.hpp>

#include "protocol/request/request.h"
#include "protocol/response/response.h"

/**
 * @brief Represents a tcp communication channel for custom protocol.
 *
 * @note sendRequest() function is thread safe
 *
 * CommunicationEndpoint is meant to have its own io_context (i.e. reside in a
 * separate thread)
 */
class CommunicationEndpoint : public std::enable_shared_from_this<CommunicationEndpoint>
{
public:
    // when the endpoint recieved a request from the net
    using request_callback_fn = std::function<void(protocol::request::Request request)>;
    // when the net sent a response to the endpoint's request, added via sendRequest()
    using response_callback_fn =
        std::function<void(std::optional<protocol::response::Response> response,
                           protocol::request::Request                  request)>;

    static std::shared_ptr<CommunicationEndpoint>
        create(boost::asio::io_context                        &context,
               boost::asio::ip::tcp::socket                    socket,
               std::function<void(protocol::request::Request)> requestCallback);

    void start();

    void sendRequest(protocol::request::Request             request,
                     response_callback_fn                   requestCallback,
                     const boost::posix_time::milliseconds &timeout =
                         boost::posix_time::milliseconds{ 1500 });

private:
    CommunicationEndpoint(
        boost::asio::io_context                        &context,
        boost::asio::ip::tcp::socket                    socket,
        std::function<void(protocol::request::Request)> requestCallback);

    struct PendingRequest : public std::enable_shared_from_this<PendingRequest>
    {
        static std::shared_ptr<PendingRequest>
            create(CommunicationEndpoint                      &endpoint,
                   const boost::posix_time::milliseconds      &timeout,
                   std::shared_ptr<protocol::request::Request> request,
                   response_callback_fn                        responseCallback);

        std::shared_ptr<protocol::request::Request> request;
        response_callback_fn                        responseCallback;
        boost::asio::deadline_timer                 timer;

    private:
        PendingRequest(CommunicationEndpoint                      &endpoint,
                       const boost::posix_time::milliseconds      &timeout,
                       std::shared_ptr<protocol::request::Request> request,
                       response_callback_fn                        responseCallback);

        void init(CommunicationEndpoint                 &endpoint,
                  const boost::posix_time::milliseconds &timeout);

        PendingRequest(const PendingRequest &)            = delete;
        PendingRequest(PendingRequest &&)                 = delete;
        PendingRequest &operator=(const PendingRequest &) = delete;
        PendingRequest &operator=(PendingRequest &&)      = delete;
    };
    friend class PendingRequest;

    boost::asio::io_context     &context_;
    boost::asio::ip::tcp::socket socket_;
    const request_callback_fn    requestCallback_;

    // request id to PendingRequest struct
    std::unordered_map<size_t, std::shared_ptr<PendingRequest>> pendingRequests_;

    CommunicationEndpoint(const CommunicationEndpoint &)            = delete;
    CommunicationEndpoint(CommunicationEndpoint &&)                 = delete;
    CommunicationEndpoint &operator=(const CommunicationEndpoint &) = delete;
    CommunicationEndpoint &operator=(CommunicationEndpoint &&)      = delete;
};
