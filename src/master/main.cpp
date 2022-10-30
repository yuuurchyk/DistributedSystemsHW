#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "constants/constants.h"
#include "iocontextpool/iocontextpool.h"
#include "logger/logger.h"
#include "socketacceptor/socketacceptor.h"

#include "masterhttpsession.h"

using namespace boost::asio;

int main()
{
    logger::setup("master");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    // -------------------------------
    // -------------------------------
    auto workersPool = IOContextPool{ constants::kMasterWorkersNum };
    workersPool.runInSeparateThreads();

    // -------------------------------
    // -------------------------------
    auto httpAcceptorContext = io_context{};
    SocketAcceptor::create(
        httpAcceptorContext,
        constants::kMasterHttpPort,
        [](ip::tcp::socket socket)
        { MasterHttpSession::create(std::move(socket))->run(); },
        workersPool)
        ->run();
    httpAcceptorContext.run();

    return 0;
}
