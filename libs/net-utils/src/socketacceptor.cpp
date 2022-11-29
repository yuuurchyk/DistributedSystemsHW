#include "net-utils/socketacceptor.h"

#include <utility>

using namespace boost::asio;
using error_code = boost::system::error_code;

std::shared_ptr<SocketAcceptor>
    SocketAcceptor::create(io_context            &acceptorContext,
                           unsigned short         port,
                           new_socket_callback_fn newSocketCallback,
                           IOContextPool         &workersPool)
{
    return std::shared_ptr<SocketAcceptor>{ new SocketAcceptor{
        acceptorContext, port, std::move(newSocketCallback), workersPool } };
}

void SocketAcceptor::run()
{
    acceptOne();
}

SocketAcceptor::SocketAcceptor(io_context            &acceptorContext,
                               unsigned short         port,
                               new_socket_callback_fn newSocketCallback,
                               IOContextPool         &workersPool)
    : acceptor_{ acceptorContext, ip::tcp::endpoint{ ip::tcp::v4(), port } },
      newSocketCallback_{ newSocketCallback == nullptr ? [](ip::tcp::socket) {} :
                                                         std::move(newSocketCallback) },
      workersPool_{ workersPool }
{
}

void SocketAcceptor::acceptOne()
{
    auto  socketOwner = std::make_unique<ip::tcp::socket>(workersPool_.getNext());
    auto &socket      = *socketOwner;

    acceptor_.async_accept(socket,
                           [this,
                            self        = shared_from_this(),
                            socketOwner = std::move(socketOwner)](const error_code &error)
                           {
                               if (error)
                               {
                                   EN_LOGE << "Could not accept incoming connection";
                                   return;
                               }

                               EN_LOGI << "Accepting incoming connection";

                               newSocketCallback_(std::move(*socketOwner));

                               acceptOne();
                           });
}
