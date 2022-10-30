#pragma once

#include <functional>
#include <memory>

#include <boost/asio.hpp>

#include "iocontextpool/iocontextpool.h"

/**
 * @brief forever accepts sockets and calls new_socket_callback_fn
 */
class SocketAcceptor : public std::enable_shared_from_this<SocketAcceptor>
{
    DISABLE_COPY_MOVE(SocketAcceptor);

public:
    using new_socket_callback_fn = std::function<void(boost::asio::ip::tcp::socket)>;

    enum class IOContextSelectionPolicy
    {
        Random,
        LeastLoaded
    };

    static std::shared_ptr<SocketAcceptor> create(boost::asio::io_context &,
                                                  unsigned short port,
                                                  new_socket_callback_fn,
                                                  IOContextPool &,
                                                  IOContextSelectionPolicy);

    void run();

private:
    SocketAcceptor(boost::asio::io_context &,
                   unsigned short port,
                   new_socket_callback_fn,
                   IOContextPool &,
                   IOContextSelectionPolicy);

    void acceptOne();

    boost::asio::ip::tcp::acceptor acceptor_;
    new_socket_callback_fn         newSocketCallback_;
    IOContextPool                 &pool_;
    const IOContextSelectionPolicy policy_;
};
