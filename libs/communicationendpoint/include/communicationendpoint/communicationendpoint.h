#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_set>

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

    void sendValidRequestImpl(std::shared_ptr<protocol::request::Request> request,
                              std::shared_ptr<response_callback_fn>  responseCallback,
                              const boost::posix_time::milliseconds &timeout);

    boost::asio::io_context     &context_;
    boost::asio::ip::tcp::socket socket_;
    const request_callback_fn    requestCallback_;

    std::unordered_set<size_t> pendingRequests_;

    struct PendingRequest
    {
        protocol::request::Request  request;
        response_callback_fn        responseCallback;
        boost::asio::deadline_timer timeoutTimer;
    };
    friend bool operator==(const CommunicationEndpoint::PendingRequest &lhs,
                           const CommunicationEndpoint::PendingRequest &rhs);

    CommunicationEndpoint(const CommunicationEndpoint &)            = delete;
    CommunicationEndpoint(CommunicationEndpoint &&)                 = delete;
    CommunicationEndpoint &operator=(const CommunicationEndpoint &) = delete;
    CommunicationEndpoint &operator=(CommunicationEndpoint &&)      = delete;
};

bool operator==(const CommunicationEndpoint::PendingRequest &lhs,
                const CommunicationEndpoint::PendingRequest &rhs);
