#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "proto/communicationendpoint.h"

int main()
{
    logger::setup("master");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "master";

    auto endpoints = std::vector<boost::asio::ip::tcp::endpoint>{};
    endpoints.emplace_back(boost::asio::ip::address::from_string("127.0.0.1"), 6000);

    auto context = boost::asio::io_context{};
    auto guard   = boost::asio::make_work_guard(context);

    auto socketEndpoint = boost::asio::ip::tcp::endpoint{ boost::asio::ip::tcp::v4(), 6000 };
    auto acceptor       = boost::asio::ip::tcp::acceptor{ context, socketEndpoint };
    auto socket         = boost::asio::ip::tcp::socket{ context };
    acceptor.accept(socketEndpoint);

    auto endpoint = Proto::CommunicationEndpoint::create(std::move(socket), /*sendTimeoutMs*/ 3000);
    endpoint->run();
    endpoint->send_addMessage(std::make_shared<Proto::Request::AddMessage>(Proto::Message{ 0, "sample message" }))
        .then(
            [](boost::future<Proto::Response::AddMessage> response)
            {
                LOGI << "recieved response";
                LOGI << response.has_value();
            });

    context.run();

    return 0;
}
