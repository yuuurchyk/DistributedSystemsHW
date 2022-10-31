#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "iocontextpool/iocontextpool.h"
#include "utils/copymove.h"

/**
 * @brief waits for master node connection and communicates with
 * master if successful. In case connection is dropped, reinitiates
 * waiting for master node.
 */
class MasterSession : public std::enable_shared_from_this<MasterSession>
{
    DISABLE_COPY_MOVE(MasterSession);

public:
    static std::shared_ptr<MasterSession>
        create(boost::asio::io_context &masterCommunicationContext,
               unsigned short           port,
               IOContextPool           &workersPool);

    void run();

private:
    MasterSession(boost::asio::io_context &, unsigned short, IOContextPool &);

    void acceptConnection();
    void createCommunicationEndpoint(boost::asio::ip::tcp::socket);
    void handleCommunicationEndpointInvalidation();

    boost::asio::io_context       &context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    IOContextPool                 &workersPool_;
};
