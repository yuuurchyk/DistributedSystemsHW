#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "communicationendpoint/communicationendpoint.h"
#include "logger/logger.h"
#include "protocol/request/pushstring.h"

using namespace boost::asio;
using error_code = boost::system::error_code;

using namespace protocol;

int main()
{
    logger::setup("client");

    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    auto context = io_context{};

    const auto endpoint =
        ip::tcp::endpoint{ ip::address::from_string("127.0.0.1"), 8001 };

    auto socket = ip::tcp::socket{ context };

    socket.connect(endpoint);
    auto com =
        CommunicationEndpoint::create(context, std::move(socket), nullptr, nullptr);

    com->run();
    com->sendRequest(protocol::request::PushString::form(1, std::string_view{ "acb" }),
                     nullptr);
    com->sendRequest(protocol::request::PushString::form(2, std::string_view{ "def" }),
                     nullptr);

    context.run();

    return 0;
}
