#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "net-utils/socketacceptor.h"
#include "proto2/endpoint.h"

int main()
{
    logger::setup("test-master");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END
    LOGI << "Hello";

    auto context   = boost::asio::io_context{};
    auto workGuard = boost::asio::make_work_guard(context);

    auto acceptor =
        boost::asio::ip::tcp::acceptor{ context, boost::asio::ip::tcp::endpoint{ boost::asio::ip::tcp::v4(), 6006 } };
    auto socket = boost::asio::ip::tcp::socket{ context };
    acceptor.accept(socket);

    auto endpoint = Proto2::Endpoint::create(context, std::move(socket), std::chrono::milliseconds{ 1500 });
    endpoint->run();

    std::string message{ "Hello from master" };
    endpoint->addMessage(0, message).then([](boost::future<Proto2::AddMessageStatus> future) { future.get(); });

    context.run();

    LOGI << "I am not here, right?";

    return 0;
}