#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "net-utils/socketacceptor.h"

#include "secondaryhttpsession.h"
#include "secondarynode.h"

struct CmdArgs
{
    std::string nodeName;

    std::string    masterIp;
    unsigned short masterCommPort{};

    size_t         httpWorkers{};
    unsigned short httpPort{};
};

CmdArgs readCmdArgs(int argc, char **argv)
{
    namespace po = boost::program_options;

    auto args = CmdArgs{};

    po::options_description desc("Options:");
    // clang-format off
    desc.add_options()
        ("help,h", "produce help message")

        ("name",         po::value<std::string>(&args.nodeName)->default_value("secondary"), "friendly name of the node")
        ("http-port",    po::value<unsigned short>(&args.httpPort)->required(),              "http port")
        ("http-workers", po::value<size_t>(&args.httpWorkers)->default_value(3),             "number of http threads to use")

        ("master-ip",        po::value<std::string>(&args.masterIp)->required(),          "ip of the master node")
        ("master-comm-port", po::value<unsigned short>(&args.masterCommPort)->required(), "master internal communication port");
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

    logger::setup(args.nodeName);
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "Using " << args.httpWorkers << " http threads";
    auto httpWorkersPool = NetUtils::IOContextPool::create(args.httpWorkers);
    httpWorkersPool->runInSeparateThreads();

    auto utilityPool = NetUtils::IOContextPool::create(1);
    utilityPool->runInSeparateThreads();
    LOGI << "Master internal communication, ip=" << args.masterIp << ", port=" << args.masterCommPort;
    auto secondaryNode = SecondaryNode::create(
        args.nodeName,
        utilityPool->getNext(),
        boost::asio::ip::tcp::endpoint{ boost::asio::ip::address::from_string(args.masterIp), args.masterCommPort });
    secondaryNode->run();

    LOGI << "Listening for http requests on port " << args.httpPort;
    auto context            = boost::asio::io_context{};
    auto httpSocketAcceptor = NetUtils::SocketAcceptor::create(
        context,
        args.httpPort,
        [weakSecondaryNode = std::weak_ptr<SecondaryNode>{ secondaryNode }](
            boost::asio::ip::tcp::socket socket, boost::asio::io_context &ioContext)
        { SecondaryHttpSession::create(ioContext, std::move(socket), weakSecondaryNode)->run(); },
        httpWorkersPool);
    httpSocketAcceptor->run();
    context.run();

    return 0;
}
