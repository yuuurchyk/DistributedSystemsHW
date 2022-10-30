#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "iocontextpool/iocontextpool.h"
#include "logger/logger.h"
#include "socketacceptor/socketacceptor.h"

#include "httpsession.h"
#include "mastersession.h"
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

    // -------------------------------
    // -------------------------------
    const auto workersNum  = 3;
    auto       workersPool = IOContextPool{ workersNum };
    workersPool.runInSeparateThreads();

    // -------------------------------
    // -------------------------------
    auto masterSessionPool = IOContextPool{ 1 };
    masterSessionPool.runInSeparateThreads();
    MasterSession::create(masterSessionPool.getNext(), /* port */ 6000, workersPool);

    // -------------------------------
    // -------------------------------
    auto httpAcceptorContext = io_context{};
    SocketAcceptor::create(
        httpAcceptorContext,
        /* port */ 8000,
        [](ip::tcp::socket socket) { HttpSession::create(std::move(socket))->run(); },
        workersPool)
        ->run();
    httpAcceptorContext.run();

    return 0;
}
