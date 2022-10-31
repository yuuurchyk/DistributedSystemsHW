#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>

#include "constants/constants.h"
#include "iocontextpool/iocontextpool.h"
#include "logger/logger.h"
#include "socketacceptor/socketacceptor.h"

#include "mastersession.h"
#include "secondaryhttpsession.h"

using namespace boost::asio;

int main(int argc, char **argv)
{
    auto id = 0;

    {
        // TODO: rewrite to accept ip addresses of secondary nodes
        namespace po = boost::program_options;

        po::options_description desc("Options:");
        desc.add_options()("id", po::value<int>(), "secondary node id (1 or 2)");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("id") == 0)
        {
            std::cerr << "id argument not found" << std::endl;
            return 0;
        }
        else
        {
            try
            {
                id = vm["id"].as<int>();

                if (id < 1 || id > 2)
                {
                    std::cerr << "wrong id " << id << ", expected 1 or 2" << std::endl;
                    return 1;
                }
            }
            catch (...)
            {
                std::cerr << "wrong format for id argument" << std::endl;
                return 1;
            }
        }
    }

    logger::setup("secondary_" + std::to_string(id));
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
    const auto communicationPort = id == 1 ? constants::kSecondary1CommunicationPort :
                                             constants::kSecondary2CommunicationPort;

    auto masterSessionPool = IOContextPool{ 1 };
    masterSessionPool.runInSeparateThreads();
    MasterSession::create(masterSessionPool.getNext(), communicationPort, workersPool)
        ->run();

    // -------------------------------
    // -------------------------------
    const auto httpPort =
        id == 1 ? constants::kSecondary1HttpPort : constants::kSecondary2HttpPort;
    auto httpAcceptorContext = io_context{};
    SocketAcceptor::create(
        httpAcceptorContext,
        httpPort,
        [](ip::tcp::socket socket)
        { SecondaryHttpSession::create(std::move(socket))->run(); },
        workersPool)
        ->run();
    httpAcceptorContext.run();

    return 0;
}
