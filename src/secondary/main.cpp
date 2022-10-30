#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "iocontextpool/iocontextpool.h"
#include "logger/logger.h"
#include "socketacceptor/socketacceptor.h"

#include "httpsession.h"
#include "messages.h"

using namespace boost::asio;

int main()
{
    logger::setup("secondary");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    messages.push_back("a");
    messages.push_back("b");

    auto acceptorContext = io_context{};

    const auto httpPort       = 8000;
    const auto httpWorkersNum = 3;
    auto       httpPool       = IOContextPool{ httpWorkersNum };
    httpPool.runInSeparateThreads();
    SocketAcceptor::create(
        acceptorContext,
        httpPort,
        [](ip::tcp::socket socket) { HttpSession::create(std::move(socket))->run(); },
        httpPool,
        SocketAcceptor::IOContextSelectionPolicy::Random)
        ->run();

    acceptorContext.run();

    httpPool.stop();

    return 0;
}
