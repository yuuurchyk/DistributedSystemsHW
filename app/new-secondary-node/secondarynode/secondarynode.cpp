#include "secondarynode/secondarynode.h"

#include "constants2/constants2.h"
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

void SecondaryNode::run()
{
    reconnectToMaster();
}

bool SecondaryNode::valid() const
{
    std::shared_lock lck{ validMutex_ };
    return valid_;
}

const SecondaryStorage &SecondaryNode::storage() const
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

            establishMasterEndpoint(std::move(*socket));
        });
}

void SecondaryNode::establishMasterEndpoint(boost::asio::ip::tcp::socket socket)
{
    masterEndpoint_ = Proto2::Endpoint::create(
        "master",
        ioContext_,
        std::move(socket),
        Constants2::kOutcomingRequestTimeout,
        Constants2::kArtificialSendDelayBounds);
    masterEndpoint_->run();

    masterEndpoint_->invalidated.connect(
        [this, weakSelf = weak_from_this()]()
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            EN_LOGW << "socket invalidated";
            invalidateMasterEndpoint();
        });

    masterEndpoint_->incoming_addMessage.connect(
        [this, weakSelf = weak_from_this()](
            size_t msgId, std::string msg, Proto2::SharedPromise<Proto2::AddMessageStatus> response)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            storage_.addMessage(msgId, std::move(msg));
            response->set_value(Proto2::AddMessageStatus::OK);
        });
    masterEndpoint_->incoming_ping.connect(
        [this, weakSelf = weak_from_this()](Proto2::Timestamp_t, Proto2::SharedPromise<Proto2::Timestamp_t> response)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            response->set_value(Proto2::getCurrentTimestamp());
        });

    askMasterForMessages();
}

void SecondaryNode::askMasterForMessages()
{
    const auto gap = storage_.getFirstGap();

    EN_LOGI << "asking master for messages, starting from messageId=" << gap;

    NetUtils::thenPost(
        masterEndpoint_->send_getMessages(gap),
        ioContext_,
        [this, gap, weakEndpoint = masterEndpoint_->weak_from_this(), weakSelf = weak_from_this()](
            boost::future<std::vector<std::string>> response)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;
            const auto endpoint = weakEndpoint.lock();
            if (endpoint == nullptr || endpoint != masterEndpoint_)
                return;

            if (response.has_value())
            {
                EN_LOGI << "Successfully retrieved messages from master, filling storage";
                storage_.addMessages(gap, response.get());
                EN_LOGI << "Making ready";
                {
                    std::unique_lock lck{ validMutex_ };
                    valid_ = true;
                }
                sendFriendlyNameToMaster();
            }
            else
            {
                EN_LOGW << "Failed to recieve messages from master, retrying";
                askMasterForMessages();
            }
        });
}

void SecondaryNode::sendFriendlyNameToMaster()
{
    EN_LOGI << "sending friendly name to master";

    NetUtils::thenPost(
        masterEndpoint_->send_secondaryNodeReady(friendlyName_),
        ioContext_,
        [this, weakEndpoint = masterEndpoint_->weak_from_this(), weakSelf = weak_from_this()](
            boost::future<void> response)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;
            const auto endpoint = weakEndpoint.lock();
            if (endpoint == nullptr || endpoint != masterEndpoint_)
                return;

            if (response.has_value())
            {
                EN_LOGI << "recieved ack from master on secondaryNodeReady request";
            }
            else
            {
                EN_LOGW << "failed to recieve ack from master on secondaryNodeReady request, retrying";
                sendFriendlyNameToMaster();
            }
        });
}

void SecondaryNode::invalidateMasterEndpoint()
{
    LOGI << "invalidating master endpoint";
    {
        std::unique_lock lck{ validMutex_ };
        valid_ = false;
    }
    masterEndpoint_ = nullptr;
    reconnectToMaster();
}
