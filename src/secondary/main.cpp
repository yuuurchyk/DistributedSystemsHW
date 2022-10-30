#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "constants/constants.h"
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

    // -------------------------------
    // -------------------------------
    auto workersPool = IOContextPool{ constants::kSecondaryWorkersNum };
    workersPool.runInSeparateThreads();

    // -------------------------------
    // -------------------------------
    auto masterSessionPool = IOContextPool{ 1 };
    masterSessionPool.runInSeparateThreads();
    MasterSession::create(
        masterSessionPool.getNext(), constants::kSecondaryCommunicationPort, workersPool)
        ->run();

    // -------------------------------
    // -------------------------------
    auto httpAcceptorContext = io_context{};
    SocketAcceptor::create(
        httpAcceptorContext,
        constants::kSecondaryHttpPort,
        [](ip::tcp::socket socket) { HttpSession::create(std::move(socket))->run(); },
        workersPool)
        ->run();
    httpAcceptorContext.run();

    return 0;
}
