#include "secondarynode.h"

#include <algorithm>
#include <utility>

#include "constants.h"

std::shared_ptr<SecondaryNode>
    SecondaryNode::create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::endpoint masterSocketEndpoint)
{
    return std::shared_ptr<SecondaryNode>{ new SecondaryNode{ ioContext, std::move(masterSocketEndpoint) } };
}

void SecondaryNode::run()
{
    reconnect();
}

std::vector<Proto::Message> SecondaryNode::getMessages()
{
    auto res = std::vector<Proto::Message>{};
    res.reserve(messages_.size());

    for (auto it = messages_.cbegin(); it != messages_.cend(); ++it)
        res.emplace_back(it->first, it->second);

    std::sort(
        res.begin(),
        res.end(),
        [](const Proto::Message &lhs, const Proto::Message &rhs) { return lhs.timestamp < rhs.timestamp; });

    return res;
}

SecondaryNode::SecondaryNode(boost::asio::io_context &ioContext, boost::asio::ip::tcp::endpoint masterSocketEndpoint)
    : ioContext_{ ioContext }, masterSocketEndpoint_{ std::move(masterSocketEndpoint) }, reconnectTimer_{ ioContext }
{
}

void SecondaryNode::reconnect()
{
    auto  socketOwner = std::make_unique<boost::asio::ip::tcp::socket>(ioContext_);
    auto &socket      = *socketOwner;

    EN_LOGI << "Reconnecting to master node...";

    socket.async_connect(
        masterSocketEndpoint_,
        [this, socketOwner = std::move(socketOwner), weakSelf = weak_from_this()](const boost::system::error_code &ec)
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
            {
                EN_LOGW << "Don't reconnect, since SecondaryNode object got destroyed";
                return;
            }

            if (ec)
            {
                EN_LOGW << "Failed to reconnect, scheduling reconnect timer";
                reconnectTimer_.expires_from_now(
                    boost::posix_time::milliseconds{ Constants::kMasterReconnectTimeoutMs });
                reconnectTimer_.async_wait(
                    [this, weakSelf = weak_from_this()](const boost::system::error_code &ec)
                    {
                        if (ec)
                            return;

                        auto self = weakSelf.lock();
                        if (self == nullptr)
                            return;

                        reconnect();
                    });
                return;
            }

            EN_LOGI << "Opened socket to master node";

            establishMasterEndpoint(std::move(*socketOwner));
        });
}

void SecondaryNode::establishMasterEndpoint(boost::asio::ip::tcp::socket socket)
{
    auto endpoint =
        Proto::CommunicationEndpoint::create(ioContext_, std::move(socket), Constants::kMasterSendTimeoutMs);

    endpoint->incoming_addMessage.connect(
        [this, weakSelf = weak_from_this()](
            std::shared_ptr<Proto::Request::AddMessage>                  request,
            std::shared_ptr<boost::promise<Proto::Response::AddMessage>> responsePromise)
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
                return;

            EN_LOGI << "Incoming add message request, msg=" << request->message;

            messages_.insert({ request->timestamp, request->message });
            responsePromise->set_value(Proto::Response::AddMessage{ Proto::Response::AddMessage::Status::OK });
        });
    endpoint->incoming_ping.connect(
        [this, weakSelf = weak_from_this()](
            std::shared_ptr<Proto::Request::Ping>                  request,
            std::shared_ptr<boost::promise<Proto::Response::Pong>> responsePromise)
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
                return;

            EN_LOGI << "Incoming ping request";

            responsePromise->set_value(Proto::Response::Pong{ Proto::getCurrentTimestamp() });
        });
    endpoint->invalidated.connect(
        [this, weakSelf = weak_from_this()]()
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
                return;

            EN_LOGI << "Endpoint invalidated";

            communicationEndpoint_ = nullptr;

            reconnect();
        });

    communicationEndpoint_ = std::move(endpoint);
    communicationEndpoint_->run();
    sendGetMessagesRequest(communicationEndpoint_);
}

void SecondaryNode::sendGetMessagesRequest(std::weak_ptr<Proto::CommunicationEndpoint> weakEndpoint)
{
    auto endpoint = weakEndpoint.lock();
    if (endpoint == nullptr)
        return;

    EN_LOGI << "Sending get messages request";

    endpoint->send_getMessages(std::make_shared<Proto::Request::GetMessages>())
        .then(
            [this, weakEndpoint, weakSelf = weak_from_this()](
                boost::future<Proto::Response::GetMessages> responseFuture)
            {
                auto self = weakSelf.lock();
                if (self == nullptr)
                    return;

                boost::asio::post(
                    ioContext_,
                    [this,
                     weakEndpoint,
                     responseFuture = std::move(responseFuture),
                     self           = shared_from_this()]() mutable
                    {
                        if (!responseFuture.has_value())
                        {
                            EN_LOGW << "Failed to recieve messages from master, retrying";
                            sendGetMessagesRequest(weakEndpoint);
                        }
                        else
                        {
                            auto messages = responseFuture.get().messages;

                            EN_LOGW << "Recieved " << messages.size() << " message(s) from master node";

                            for (auto &message : messages)
                                messages_.insert({ message.timestamp, std::move(message.message) });
                            sendSecondaryNodeReadyRequest(weakEndpoint);
                        }
                    });
            });
}

void SecondaryNode::sendSecondaryNodeReadyRequest(std::weak_ptr<Proto::CommunicationEndpoint> weakEndpoint)
{
    auto endpoint = weakEndpoint.lock();
    if (endpoint == nullptr)
        return;

    EN_LOGI << "Sending secondary node ready request";

    endpoint->send_secondaryNodeReady(std::make_shared<Proto::Request::SecondaryNodeReady>())
        .then(
            [this, weakEndpoint, weakSelf = weak_from_this()](
                boost::future<Proto::Response::SecondaryNodeReady> responseFuture)
            {
                auto self = weakSelf.lock();
                if (self == nullptr)
                    return;

                boost::asio::post(
                    ioContext_,
                    [this,
                     weakEndpoint,
                     responseFuture = std::move(responseFuture),
                     self           = shared_from_this()]() mutable
                    {
                        if (!responseFuture.has_value())
                        {
                            EN_LOGW << "Failed to recieve acknowledgment from master, retrying";
                            sendGetMessagesRequest(weakEndpoint);
                        }
                        else
                        {
                            EN_LOGI << "Secondary node is considered ready";
                        }
                    });
            });
}
