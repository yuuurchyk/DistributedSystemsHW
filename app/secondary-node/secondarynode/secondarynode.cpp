#include "secondarynode/secondarynode.h"

#include "net-utils/launchwithdelay.h"
#include "net-utils/thenpost.h"
#include "utils/signal.h"

#include <utility>

using error_code = boost::system::error_code;

std::shared_ptr<SecondaryNode> SecondaryNode::create(
    std::string                               friendlyName,
    boost::asio::io_context                  &executionContext,
    boost::asio::ip::tcp::endpoint            masterAddress,
    std::chrono::duration<size_t, std::milli> masterReconnectInterval)
{
    return std::shared_ptr<SecondaryNode>{ new SecondaryNode{
        std::move(friendlyName), executionContext, std::move(masterAddress), std::move(masterReconnectInterval) } };
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
    boost::asio::io_context                  &executionContext,
    boost::asio::ip::tcp::endpoint            masterAddress,
    std::chrono::duration<size_t, std::milli> masterReconnectInterval)
    : friendlyName_{ std::move(friendlyName) },
      executionContext_{ executionContext },
      masterAddress_{ std::move(masterAddress) },
      masterReconnectInterval_{ masterReconnectInterval.count() }
{
}

void SecondaryNode::reconnectToMaster()
{
    EN_LOGI << "reconnecting to master node";

    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(executionContext_);

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
                    executionContext_,
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
    session_ = {};

    std::unique_lock lck{ operationalMutex_ };
    operational_ = false;
}

void SecondaryNode::runMasterSession(boost::asio::ip::tcp::socket socket)
{
    disconnectMasterSession();

    session_ = MasterSession::create(friendlyName_, executionContext_, std::move(socket), storage_);

    session_->invalidated.connect(
        Utils::slot_type<>{ &SecondaryNode::onSessionInvalidated, this }.track_foreign(weak_from_this()));
    session_->operational.connect(
        Utils::slot_type<>{ &SecondaryNode::onSessionOperational, this }.track_foreign(weak_from_this()));

    session_->run();
}

void SecondaryNode::onSessionInvalidated()
{
    EN_LOGI << "master session invalidated";

    disconnectMasterSession();
    reconnectToMaster();
}

void SecondaryNode::onSessionOperational()
{
    EN_LOGI << "master session operational";

    std::unique_lock lck{ operationalMutex_ };
    operational_ = true;
}
