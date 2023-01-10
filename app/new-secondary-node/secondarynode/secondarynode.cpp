#include "secondarynode/secondarynode.h"

#include "net-utils/launchwithdelay.h"
#include "net-utils/thenpost.h"

#include <utility>

using error_code = boost::system::error_code;

std::shared_ptr<SecondaryNode> SecondaryNode::create(
    std::string                               friendlyName,
    boost::asio::io_context                  &ioContext,
    boost::asio::ip::tcp::endpoint            masterAddress,
    std::chrono::duration<size_t, std::milli> masterReconnectInterval)
{
    return std::shared_ptr<SecondaryNode>{ new SecondaryNode{
        std::move(friendlyName), ioContext, std::move(masterAddress), std::move(masterReconnectInterval) } };
}

SecondaryNode::~SecondaryNode()
{
    disconnectMasterSession();
}

void SecondaryNode::run()
{
    reconnectToMaster();
}

bool SecondaryNode::operational() const
{
    std::shared_lock lck{ operationalMutex_ };
    return operational_;
}

const Storage &SecondaryNode::storage() const
{
    return storage_;
}

SecondaryNode::SecondaryNode(
    std::string                               friendlyName,
    boost::asio::io_context                  &ioContext,
    boost::asio::ip::tcp::endpoint            masterAddress,
    std::chrono::duration<size_t, std::milli> masterReconnectInterval)
    : friendlyName_{ std::move(friendlyName) },
      ioContext_{ ioContext },
      masterAddress_{ std::move(masterAddress) },
      masterReconnectInterval_{ masterReconnectInterval.count() }
{
}

void SecondaryNode::reconnectToMaster()
{
    EN_LOGI << "reconnecting to master node";

    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext_);

    socket->async_connect(
        masterAddress_,
        [this, socket, weakSelf = weak_from_this()](const error_code &ec)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (ec)
            {
                EN_LOGI << "failed to connect to master, schedule reconnect";
                NetUtils::launchWithDelay(
                    ioContext_,
                    masterReconnectInterval_,
                    [this, weakSelf = weak_from_this()]()
                    {
                        const auto self = weakSelf.lock();
                        if (self != nullptr)
                            reconnectToMaster();
                    });
                return;
            }

            EN_LOGI << "connected to master node, establising endpoint";

            runMasterSession(std::move(*socket));
        });
}

void SecondaryNode::disconnectMasterSession()
{
    {
        std::unique_lock lck{ operationalMutex_ };
        operational_ = false;
    }
    for (auto &connection : sessionConnections_)
        connection.disconnect();
    session_ = {};
}

void SecondaryNode::runMasterSession(boost::asio::ip::tcp::socket socket)
{
    disconnectMasterSession();

    session_ = MasterSession::create(friendlyName_, ioContext_, std::move(socket), storage_);

    auto invalidatedConnection = session_->invalidated.connect(
        [this]()
        {
            EN_LOGI << "master session invalidated";
            disconnectMasterSession();
            reconnectToMaster();
        });

    auto operationalConnection = session_->operational.connect(
        [this]()
        {
            EN_LOGI << "master session operational";
            {
                std::unique_lock lck{ operationalMutex_ };
                operational_ = true;
            }
        });

    sessionConnections_.push_back(std::move(invalidatedConnection));
    sessionConnections_.push_back(std::move(operationalConnection));

    session_->run();
}
