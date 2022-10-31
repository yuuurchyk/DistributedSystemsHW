#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "constants/constants.h"
#include "iocontextpool/iocontextpool.h"
#include "logger/logger.h"
#include "secondarysession.h"
#include "socketacceptor/socketacceptor.h"

#include "masterhttpsession.h"
#include "secondariespool.h"

#include "protocol/request/pushstring.h"

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

    auto endpoints = std::vector<ip::tcp::endpoint>{};
    endpoints.emplace_back(ip::address::from_string("127.0.0.1"),
                           constants::kSecondary1CommunicationPort);
    endpoints.emplace_back(ip::address::from_string("127.0.0.1"),
                           constants::kSecondary2CommunicationPort);

    auto secondariesPoolContext = IOContextPool{ 1 };
    secondariesPoolContext.runInSeparateThreads();
    auto secondariesContexts = IOContextPool{ 2 };
    secondariesContexts.runInSeparateThreads();

    auto secondariesPool = SecondariesPool::create(
        secondariesPoolContext.getNext(), endpoints, secondariesContexts);

    // -------------------------------
    // -------------------------------
    auto httpAcceptorContext = io_context{};
    SocketAcceptor::create(
        httpAcceptorContext,
        constants::kMasterHttpPort,
        [secondariesPool](ip::tcp::socket socket) {
            MasterHttpSession::create(std::move(secondariesPool), std::move(socket))
                ->run();
        },
        workersPool)
        ->run();
    httpAcceptorContext.run();

    return 0;
}
