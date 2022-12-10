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

    auto context = boost::asio::io_context{};
    auto guard   = boost::asio::make_work_guard(context);

    auto socketEndpoint = boost::asio::ip::tcp::endpoint{ boost::asio::ip::tcp::v4(), 6000 };
    auto acceptor       = boost::asio::ip::tcp::acceptor{ context, socketEndpoint };
    auto socket         = boost::asio::ip::tcp::socket{ context };
    acceptor.accept(socket);

    auto endpoint = Proto::CommunicationEndpoint::create(context, std::move(socket), /*sendTimeoutMs*/ 3000);
    endpoint->run();
    endpoint->send_addMessage(std::make_shared<Proto::Request::AddMessage>(0, "sample message"))
        .then(
            [](boost::future<Proto::Response::AddMessage> responseFuture)
            {
                LOGI << "recieved response";
                LOGI << responseFuture.has_value();
            });
    endpoint->incoming_getMessages.connect(
        [](std::shared_ptr<Proto::Request::GetMessages>                  request,
           std::shared_ptr<boost::promise<Proto::Response::GetMessages>> promise)
        {
            LOGI << "Incoming get messages request";

            auto response = Proto::Response::GetMessages{};
            response.messages.push_back(Proto::Message{ 0, "sample1" });
            response.messages.push_back(Proto::Message{ 1, "sample2" });

            promise->set_value(std::move(response));
        });

    context.run();

    return 0;
}
