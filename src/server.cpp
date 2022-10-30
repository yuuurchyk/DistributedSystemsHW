#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "communicationendpoint/communicationendpoint.h"
#include "logger/logger.h"

using namespace boost::asio;
using error_code = boost::system::error_code;

int main()
{
    logger::setup("server");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    auto context = io_context{};

    auto acceptor =
        ip::tcp::acceptor{ context, ip::tcp::endpoint{ ip::tcp::v4(), 8001 } };

    auto socket = ip::tcp::socket{ context };
    acceptor.accept(socket);

    auto com =
        CommunicationEndpoint::create(context, std::move(socket), nullptr, nullptr);
    com->run();

    context.run();

    return 0;
}
