#include "net-utils/socketacceptor.h"

#include <utility>

using namespace boost::asio;
using error_code = boost::system::error_code;

namespace NetUtils
{
std::shared_ptr<SocketAcceptor> SocketAcceptor::create(
    io_context                                   &acceptorContext,
    unsigned short                                port,
    RandomIOContextPolicy::new_socket_callback_fn newSocketCallback,
    std::shared_ptr<IOContextPool>                workers)
{
    return std::shared_ptr<SocketAcceptor>{ new SocketAcceptor{
        acceptorContext, port, std::move(newSocketCallback), std::move(workers) } };
}
std::shared_ptr<SocketAcceptor> SocketAcceptor::create(
    io_context                                        &acceptorContext,
    unsigned short                                     port,
    LeastLoadedIOContextPolicy::new_socket_callback_fn newSocketCallback,
    std::shared_ptr<IOContextPool>                     workers)
{
    return std::shared_ptr<SocketAcceptor>{ new SocketAcceptor{
        acceptorContext, port, std::move(newSocketCallback), std::move(workers) } };
}

SocketAcceptor::SocketAcceptor(
    io_context                                   &acceptorContext,
    unsigned short                                port,
    RandomIOContextPolicy::new_socket_callback_fn newSocketCallback,
    std::shared_ptr<IOContextPool>                workersPool)
    : acceptor_{ acceptorContext, ip::tcp::endpoint{ ip::tcp::v4(), port } },
      randomCallback_{ std::move(newSocketCallback) },
      workersPool_{ std::move(workersPool) }
{
}
SocketAcceptor::SocketAcceptor(
    io_context                                        &acceptorContext,
    unsigned short                                     port,
    LeastLoadedIOContextPolicy::new_socket_callback_fn newSocketCallback,
    std::shared_ptr<IOContextPool>                     workersPool)
    : acceptor_{ acceptorContext, ip::tcp::endpoint{ ip::tcp::v4(), port } },
      leastLoadedCallback_{ std::move(newSocketCallback) },
      workersPool_{ std::move(workersPool) }
{
}

void SocketAcceptor::run()
{
    if (randomCallback_ != nullptr)
        return acceptOne(RandomIOContextPolicy{});
    if (leastLoadedCallback_ != nullptr)
        return acceptOne(LeastLoadedIOContextPolicy{});

    EN_LOGE << "No suitable policy to choose on run()";
}

void SocketAcceptor::acceptOne(RandomIOContextPolicy)
{
    auto &ioContext = workersPool_->getNext();

    auto  socketOwner = std::make_unique<ip::tcp::socket>(ioContext);
    auto &socket      = *socketOwner;

    acceptor_.async_accept(
        socket,
        [this, &ioContext, socketOwner = std::move(socketOwner), self = shared_from_this()](const error_code &ec)
        {
            if (ec)
            {
                EN_LOGE << "Could not accept incoming connection";
                return;
            }

            EN_LOGI << "Accepting incoming connection";

            randomCallback_(std::move(*socketOwner), ioContext);

            acceptOne(RandomIOContextPolicy{});
        });
}

void SocketAcceptor::acceptOne(LeastLoadedIOContextPolicy)
{
    auto  loadGuard = workersPool_->getLeastLoaded();
    auto &ioContext = loadGuard->ioContext_;

    auto  socketOwner = std::make_unique<ip::tcp::socket>(loadGuard->ioContext_);
    auto &socket      = *socketOwner;

    acceptor_.async_accept(
        socket,
        [this,
         &ioContext,
         socketOwner = std::move(socketOwner),
         loadGuard   = std::move(loadGuard),
         self        = shared_from_this()](const error_code &ec) mutable
        {
            if (ec)
            {
                EN_LOGE << "Could not accept incoming conection";
                return;
            }

            EN_LOGI << "Accepting incoming connection";

            leastLoadedCallback_(std::move(*socketOwner), std::move(loadGuard));

            acceptOne(LeastLoadedIOContextPolicy{});
        });
}

}    // namespace NetUtils
