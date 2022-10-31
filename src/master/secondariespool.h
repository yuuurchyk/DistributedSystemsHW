#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "iocontextpool/iocontextpool.h"
#include "protocol/request/request.h"
#include "utils/copymove.h"

#include "secondarysession.h"

class SecondariesPool : public std::enable_shared_from_this<SecondariesPool>
{
    DISABLE_COPY_MOVE(SecondariesPool);

public:
    static std::shared_ptr<SecondariesPool>
        create(boost::asio::io_context &context,
               std::vector<boost::asio::ip::tcp::endpoint>,
               IOContextPool &secondariesContexts);

    using response_callback_fn = std::function<void(
        std::vector<std::optional<protocol::response::Response>> response,
        protocol::request::Request                               request)>;

    /**
     * @note request id will be transformed for each secondary node, it can
     * be arbitrary here
     */
    void sendRequest(protocol::request::Request, response_callback_fn);

private:
    SecondariesPool(boost::asio::io_context &,
                    std::vector<boost::asio::ip::tcp::endpoint>,
                    IOContextPool &);

    struct PendingRequest;

    void handleResponseFromSecondary(std::shared_ptr<PendingRequest>,
                                     std::optional<protocol::response::Response>);

    struct PendingRequest : public std::enable_shared_from_this<PendingRequest>
    {
        DISABLE_COPY_MOVE(PendingRequest);

        static std::shared_ptr<PendingRequest> create(protocol::request::Request,
                                                      response_callback_fn);

        protocol::request::Request                               request;
        response_callback_fn                                     responseCallback;
        size_t                                                   responsesRecieved{};
        std::vector<std::optional<protocol::response::Response>> responses{};

    private:
        PendingRequest(protocol::request::Request, response_callback_fn);
    };

    boost::asio::io_context                       &context_;
    std::vector<std::shared_ptr<SecondarySession>> secondaries_;
};
