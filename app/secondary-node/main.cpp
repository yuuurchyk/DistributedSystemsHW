#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "net-utils/socketacceptor.h"

#include "secondaryhttpsession.h"
#include "secondarynode.h"

int main()
{
    logger::setup("secondary-node");    // TODO: distinguish between secondary nodes
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    auto httpWorkersPool = NetUtils::IOContextPool::create(3);
    httpWorkersPool->runInSeparateThreads();

    auto utilityPool = NetUtils::IOContextPool::create(1);
    utilityPool->runInSeparateThreads();
    auto secondaryNode = SecondaryNode::create(
        utilityPool->getNext(),
        boost::asio::ip::tcp::endpoint{ boost::asio::ip::address::from_string("127.0.0.1"), 6006 });
    secondaryNode->run();

    auto context            = boost::asio::io_context{};
    auto httpSocketAcceptor = NetUtils::SocketAcceptor::create(
        context,
        8081,
        [weakSecondaryNode = std::weak_ptr<SecondaryNode>{ secondaryNode }](
            boost::asio::ip::tcp::socket socket, boost::asio::io_context &ioContext)
        { SecondaryHttpSession::create(ioContext, std::move(socket), weakSecondaryNode)->run(); },
        httpWorkersPool);
    httpSocketAcceptor->run();
    context.run();

    return 0;
}
