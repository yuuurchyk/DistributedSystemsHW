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
 *
 * @note the policy of io context selection can be specified during construction
 * (i.e. by specifying the appropriate new socket callback function)
 */
class SocketAcceptor : public std::enable_shared_from_this<SocketAcceptor>, private logger::Entity<SocketAcceptor>
{
    DISABLE_COPY_MOVE(SocketAcceptor);

public:
    struct RandomIOContextPolicy
    {
        using new_socket_callback_fn = std::function<void(boost::asio::ip::tcp::socket, boost::asio::io_context &)>;
    };
    struct LeastLoadedIOContextPolicy
    {
        using new_socket_callback_fn =
            std::function<void(boost::asio::ip::tcp::socket, std::unique_ptr<IOContextPool::LoadGuard>)>;
    };

    [[nodiscard]] static std::shared_ptr<SocketAcceptor> create(
        boost::asio::io_context                      &acceptorContext,
        unsigned short                                port,
        RandomIOContextPolicy::new_socket_callback_fn newSocketCallback,
        std::shared_ptr<IOContextPool>                workersPool);
    [[nodiscard]] static std::shared_ptr<SocketAcceptor> create(
        boost::asio::io_context                           &acceptorContext,
        unsigned short                                     port,
        LeastLoadedIOContextPolicy::new_socket_callback_fn newSocketCallback,
        std::shared_ptr<IOContextPool>                     workersPool);

    void run();

private:
    SocketAcceptor(
        boost::asio::io_context &,
        unsigned short port,
        RandomIOContextPolicy::new_socket_callback_fn,
        std::shared_ptr<IOContextPool>);
    SocketAcceptor(
        boost::asio::io_context &,
        unsigned short port,
        LeastLoadedIOContextPolicy::new_socket_callback_fn,
        std::shared_ptr<IOContextPool>);

    void acceptOne(RandomIOContextPolicy);
    void acceptOne(LeastLoadedIOContextPolicy);

    boost::asio::ip::tcp::acceptor acceptor_;

    RandomIOContextPolicy::new_socket_callback_fn      randomCallback_{};
    LeastLoadedIOContextPolicy::new_socket_callback_fn leastLoadedCallback_{};

    std::shared_ptr<IOContextPool> workersPool_;
};

}    // namespace NetUtils
