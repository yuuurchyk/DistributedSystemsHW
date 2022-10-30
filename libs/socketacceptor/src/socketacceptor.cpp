#include "socketacceptor/socketacceptor.h"

#include <utility>

#include "logger/logger.h"

using namespace boost::asio;
using error_code = boost::system::error_code;

std::shared_ptr<SocketAcceptor>
    SocketAcceptor::create(io_context              &context,
                           unsigned short           port,
                           new_socket_callback_fn   newSocketCallback,
                           IOContextPool           &pool,
                           IOContextSelectionPolicy policy)
{
    return std::shared_ptr<SocketAcceptor>{ new SocketAcceptor{
        context, port, std::move(newSocketCallback), pool, policy } };
}

void SocketAcceptor::run()
{
    acceptOne();
}

SocketAcceptor::SocketAcceptor(io_context              &context,
                               unsigned short           port,
                               new_socket_callback_fn   newSocketCallback,
                               IOContextPool           &pool,
                               IOContextSelectionPolicy policy)
    : acceptor_{ context, ip::tcp::endpoint{ ip::tcp::v4(), port } },
      newSocketCallback_{ newSocketCallback == nullptr ? [](ip::tcp::socket) {} :
                                                         std::move(newSocketCallback) },
      pool_{ pool },
      policy_{ policy }
{
}

void SocketAcceptor::acceptOne()
{
    auto &socketContext = [this]() -> io_context &
    {
        switch (policy_)
        {
        case IOContextSelectionPolicy::LeastLoaded:
            return pool_.getLeastLoaded();
        case IOContextSelectionPolicy::Random:    // fall-though
        default:
            return pool_.getNext();
        }
    }();

    auto  socketOwner = std::make_unique<ip::tcp::socket>(socketContext);
    auto &socket      = *socketOwner;

    acceptor_.async_accept(socket,
                           [this,
                            self        = shared_from_this(),
                            socketOwner = std::move(socketOwner)](const error_code &error)
                           {
                               if (error)
                               {
                                   LOGE << "Could not accept incoming connection";
                                   return;
                               }

                               LOGI << "Accepting incoming connection";

                               newSocketCallback_(std::move(*socketOwner));

                               acceptOne();
                           });
}
