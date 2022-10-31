#pragma once

#include <memory>
#include <optional>
#include <shared_mutex>

#include <boost/asio.hpp>

#include "communicationendpoint/communicationendpoint.h"
#include "logger/logger.h"
#include "protocol/request/request.h"
#include "protocol/response/response.h"
#include "utils/copymove.h"

class SecondarySession : public std::enable_shared_from_this<SecondarySession>,
                         private logger::IdEntity<SecondarySession>
{
public:
    static std::shared_ptr<SecondarySession>
        create(boost::asio::io_context &secondaryCommunicationContext,
               boost::asio::ip::tcp::endpoint,
               boost::posix_time::milliseconds reconnectTimeout);

    using response_callback_fn =
        std::function<void(std::optional<protocol::response::Response> response,
                           protocol::request::Request                  request)>;

    void run();

    size_t getNextRequestId();
    void   sendRequest(protocol::request::Request request,
                       response_callback_fn       responseCallback);

private:
    SecondarySession(boost::asio::io_context &,
                     boost::asio::ip::tcp::endpoint,
                     boost::posix_time::milliseconds);

    void connect();
    void scheduleReconnect();

    void createCommunicationEndpoint(boost::asio::ip::tcp::socket);
    void handleCommunicationEndpointInvalidation();

    boost::asio::io_context              &context_;
    const boost::asio::ip::tcp::endpoint  endpoint_;
    const boost::posix_time::milliseconds reconnectTimeout_;
    boost::asio::deadline_timer           reconnectTimer_;

    std::shared_mutex                      communicationEndpointMutex_;
    std::shared_ptr<CommunicationEndpoint> communicationEndpoint_;
};
