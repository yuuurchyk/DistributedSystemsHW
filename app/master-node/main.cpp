#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "net-utils/socketacceptor.h"

#include "masterhttpsession.h"
#include "masternode.h"

int main()
{
    logger::setup("master-node");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    auto secondariesPool = NetUtils::IOContextPool::create(3);
    secondariesPool->runInSeparateThreads();

    auto httpWorkersPool = NetUtils::IOContextPool::create(3);
    httpWorkersPool->runInSeparateThreads();

    auto utilityPool = NetUtils::IOContextPool::create(1);
    utilityPool->runInSeparateThreads();
    auto masterNode = MasterNode::create(utilityPool->getNext(), 6006, secondariesPool);
    masterNode->run();

    auto context            = boost::asio::io_context{};
    auto httpSocketAcceptor = NetUtils::SocketAcceptor::create(
        context,
        8080,
        [weakMasterNode = std::weak_ptr<MasterNode>{ masterNode }](
            boost::asio::ip::tcp::socket socket, boost::asio::io_context &ioContext)
        { MasterHttpSession::create(ioContext, std::move(socket), weakMasterNode)->run(); },
        httpWorkersPool);
    httpSocketAcceptor->run();
    context.run();

    return 0;
}
