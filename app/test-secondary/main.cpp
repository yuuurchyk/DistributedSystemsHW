#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "proto2/endpoint.h"

int main()
{
    logger::setup("test-secondary", logger::Severity::Info);
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END
    LOGI << "Hello";

    auto context   = boost::asio::io_context{};
    auto workGuard = boost::asio::make_work_guard(context);

    auto socket = boost::asio::ip::tcp::socket{ context };
    socket.connect(boost::asio::ip::tcp::endpoint{ boost::asio::ip::address::from_string("127.0.0.1"), 6006 });

    auto endpoint = Proto2::Endpoint::create(
        context,
        std::move(socket),
        Proto2::duration_milliseconds_t{ 1500 },
        std::make_pair(Proto2::duration_milliseconds_t{ 1000 }, Proto2::duration_milliseconds_t{ 2000 }));

    endpoint->incoming_addMessage.connect(
        [endpoint](size_t id, std::string message, Proto2::SharedPromise<Proto2::AddMessageStatus> response)
        {
            LOGI << "Incoming add message: id =" << id << ", message = " << message;
            response->set_value(Proto2::AddMessageStatus::NOT_ALLOWED);

            endpoint->send_secondaryNodeReady("here is a secondary name");
        });
    endpoint->run();

    context.run();

    LOGI << "I am not here, right?";

    return 0;
}
