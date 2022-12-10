#include <iostream>

#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "proto/communicationendpoint.h"
#include "proto/proto.h"

int main()
{
    logger::setup("secondary");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "secondary";

    auto context = boost::asio::io_context{};
    auto guard   = boost::asio::make_work_guard(context);

    auto socketEndpoint = boost::asio::ip::tcp::endpoint{ boost::asio::ip::tcp::v4(), 6000 };
    auto socket         = boost::asio::ip::tcp::socket{ context };
    socket.connect(socketEndpoint);

    auto endpoint = Proto::CommunicationEndpoint::create(context, std::move(socket), /*sendTimeoutMs*/ 3000);
    endpoint->run();
    endpoint->incoming_addMessage.connect(
        [](std::shared_ptr<Proto::Request::AddMessage>,
           std::shared_ptr<boost::promise<Proto::Response::AddMessage>> promise)
        {
            LOGI << "Recieved incoming add message request, sending response";
            auto response = Proto::Response::AddMessage{ Proto::Response::AddMessage::Status::OK };
            promise->set_value(std::move(response));
        });

    context.run();

    return 0;
}
