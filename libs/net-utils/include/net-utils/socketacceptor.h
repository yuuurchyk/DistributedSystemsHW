#pragma once

#include <functional>
#include <memory>

#include <boost/asio.hpp>

#include "logger/logger.h"

#include "iocontextpool.h"

namespace NetUtils
{
/**
 * @brief forever accepts sockets and calls new_socket_callback_fn
 */
class SocketAcceptor : public std::enable_shared_from_this<SocketAcceptor>,
                       public logger::Entity<SocketAcceptor>
{
    DISABLE_COPY_MOVE(SocketAcceptor);

public:
    using new_socket_callback_fn = std::function<void(boost::asio::ip::tcp::socket)>;

    static std::shared_ptr<SocketAcceptor>
        create(boost::asio::io_context &acceptorContext,
               unsigned short           port,
               new_socket_callback_fn,
               IOContextPool &workersPool);

    void run();

private:
    SocketAcceptor(boost::asio::io_context &,
                   unsigned short port,
                   new_socket_callback_fn,
                   IOContextPool &);

    void acceptOne();

    boost::asio::ip::tcp::acceptor acceptor_;
    new_socket_callback_fn         newSocketCallback_;
    IOContextPool                 &workersPool_;
};

}    // namespace NetUtils
