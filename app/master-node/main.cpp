#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "net-utils/socketacceptor.h"

#include "httpsession/httpsession.h"
#include "masternode/masternode.h"

struct CmdArgs
{
    std::string nodeName;

    unsigned short httpPort{};
    unsigned short commPort{};

    size_t httpWorkers{};
    size_t commWorkers{};
};

CmdArgs readCmdArgs(int argc, char **argv)
{
    namespace po = boost::program_options;

    auto args = CmdArgs{};

    po::options_description desc("Options:");
    // clang-format off
    desc.add_options()
        ("help,h", "produce help message")

        ("name",         po::value<std::string>(&args.nodeName),                 "friendly name of the node")

        ("http-port",    po::value<unsigned short>(&args.httpPort)->required(),  "http port")
        ("comm-port",    po::value<unsigned short>(&args.commPort)->required(),  "internal communication port")

        ("http-workers", po::value<size_t>(&args.httpWorkers)->default_value(3), "number of http threads to use")
        ("comm-workers", po::value<size_t>(&args.commWorkers)->default_value(3), "number of internal communication threads to use");
    // clang-format on

    po::variables_map vm;

    auto showHelp = [&]()
    {
        std::cout << desc << std::endl;
        std::exit(0);
    };

    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
            showHelp();

        return args;
    }
    catch (const po::required_option &e)
    {
        if (vm.count("help"))
            showHelp();
        else
            throw e;
    }

    throw std::runtime_error{ "internal error" };
}

int main(int argc, char **argv)
{
    const auto args = readCmdArgs(argc, argv);

    logger::setup(args.nodeName.empty() ? std::string{ "master-node" } : args.nodeName, logger::Severity::Info);
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "Using " << args.httpWorkers << " http threads";
    auto httpWorkersPool = NetUtils::IOContextPool::create(3);
    httpWorkersPool->runInSeparateThreads();

    auto utilityPool = NetUtils::IOContextPool::create(2);
    utilityPool->runInSeparateThreads();

    auto masterNode = MasterNode::create(utilityPool->getNext());

    LOGI << "Internal communication port=" << args.commPort;
    LOGI << "Using " << args.commWorkers << " internal communication threads";
    auto secondariesPool = NetUtils::IOContextPool::create(args.commWorkers);
    secondariesPool->runInSeparateThreads();

    auto secondariesAcceptor = NetUtils::SocketAcceptor::create(
        utilityPool->getNext(),
        args.commPort,
        [weakMasterNode = masterNode->weak_from_this()](
            boost::asio::ip::tcp::socket socket, std::unique_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard)
        {
            auto masterNode = weakMasterNode.lock();
            if (masterNode == nullptr)
                return;

            auto &secondaryContext = loadGuard->ioContext_;

            masterNode->addSecondary(secondaryContext, std::move(socket), std::move(loadGuard));
        },
        secondariesPool);
    secondariesAcceptor->run();

    LOGI << "Listening for http requests on port " << args.httpPort;
    auto context            = boost::asio::io_context{};
    auto httpSocketAcceptor = NetUtils::SocketAcceptor::create(
        context,
        args.httpPort,
        [weakMasterNode =
             masterNode->weak_from_this()](boost::asio::ip::tcp::socket socket, boost::asio::io_context &ioContext)
        { MasterHttpSession::create(ioContext, std::move(socket), weakMasterNode)->run(); },
        httpWorkersPool);
    httpSocketAcceptor->run();
    context.run();

    return 0;
}
