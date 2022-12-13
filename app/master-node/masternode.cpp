#include "masternode.h"

#include <utility>

#include "proto/communicationendpoint.h"

#include "compositeaddmessagerequest.h"
#include "compositepingrequest.h"
#include "constants.h"

std::shared_ptr<MasterNode> MasterNode::create(
    boost::asio::io_context                 &ioContext,
    unsigned short                           port,
    std::shared_ptr<NetUtils::IOContextPool> secondariesPool)
{
    return std::shared_ptr<MasterNode>{ new MasterNode{ ioContext, port, std::move(secondariesPool) } };
}

void MasterNode::run()
{
    acceptor_->run();
}

boost::future<bool> MasterNode::addMessage(boost::asio::io_context &ioContext, std::string message, size_t writeConcern)
{
    const auto timestamp = Proto::getCurrentTimestamp();

    EN_LOGI << "addMessage(msg=" << message << ", writeConcern=" << writeConcern << ")";

    messages_.insert({ timestamp, message });

    auto request = CompositeAddMessageRequest::create(
        ioContext,
        weak_from_this(),
        Proto::Message{ timestamp, std::move(message) },
        writeConcern <= 1 ? size_t{} : writeConcern - 1);

    request->run();

    return request->getFuture();
}

boost::future<std::optional<std::string>> MasterNode::pingSecondaries(boost::asio::io_context &ioContext)
{
    EN_LOGI << "pingSecondaries()";

    auto request = CompositePingRequest::create(ioContext, weak_from_this());

    request->run();

    return request->getFuture();
}

std::vector<Proto::Message> MasterNode::getMessages()
{
    auto res = std::vector<Proto::Message>{};
    res.reserve(messages_.size());

    for (auto it = messages_.cbegin(); it != messages_.cend(); ++it)
        res.emplace_back(it->first, it->second);

    return res;
}

MasterNode::MasterNode(
    boost::asio::io_context                 &ioContext,
    unsigned short                           port,
    std::shared_ptr<NetUtils::IOContextPool> pool)
    : ioContext_{ ioContext }
{
    acceptor_ = NetUtils::SocketAcceptor::create(
        ioContext,
        port,
        [this](boost::asio::ip::tcp::socket socket, std::unique_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard)
        { addSecondary(std::move(socket), std::move(loadGuard)); },
        std::move(pool));
}

size_t MasterNode::getNextSecondaryId()
{
    return secondaryCounter_.fetch_add(1, std::memory_order_relaxed);
}

void MasterNode::addSecondary(
    boost::asio::ip::tcp::socket                        socket,
    std::unique_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard)
{
    const auto id = getNextSecondaryId();

    auto endpoint = Proto::CommunicationEndpoint::create(
        loadGuard->ioContext_, std::move(socket), Constants::kSecondarySendTimeoutMs);

    endpoint->invalidated.connect(
        [this, id, weakSelf = weak_from_this()]()
        {
            auto self = weakSelf.lock();
            if (self != nullptr)
                removeSecondary(id);
        });
    endpoint->incoming_secondaryNodeReady.connect(
        [this, id, weakSelf = weak_from_this()](
            std::shared_ptr<Proto::Request::SecondaryNodeReady>,
            std::shared_ptr<boost::promise<Proto::Response::SecondaryNodeReady>> responsePromise)
        {
            auto self = weakSelf.lock();
            if (self != nullptr)
                markRegistered(id, std::move(responsePromise));
        });
    endpoint->incoming_addMessage.connect(
        [](std::shared_ptr<Proto::Request::AddMessage>,
           std::shared_ptr<boost::promise<Proto::Response::AddMessage>> responsePromise)
        {
            auto response   = Proto::Response::AddMessage{};
            response.status = Proto::Response::AddMessage::Status::NOT_ALLOWED;
            responsePromise->set_value(std::move(response));
        });
    endpoint->incoming_getMessages.connect(
        [this, weakSelf = weak_from_this()](
            std::shared_ptr<Proto::Request::GetMessages>,
            std::shared_ptr<boost::promise<Proto::Response::GetMessages>> responsePromise)
        {
            auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            auto response = Proto::Response::GetMessages{ getMessages() };
            responsePromise->set_value(std::move(response));
        });

    auto secondary = std::make_unique<Secondary>();

    secondary->id        = id;
    secondary->endpoint  = std::move(endpoint);
    secondary->loadGuard = std::move(loadGuard);

    auto &endpointRef = *secondary->endpoint;

    {
        EN_LOGI << "Adding secondary, id=" << id;
        std::unique_lock<std::shared_mutex> lck{ secondariesMutex_ };
        secondaries_.insert({ id, std::move(secondary) });
    }

    endpointRef.run();
}

void MasterNode::markRegistered(
    size_t                                                               secondaryId,
    std::shared_ptr<boost::promise<Proto::Response::SecondaryNodeReady>> responsePromise)
{
    EN_LOGI << "markRegistered(secondaryId=" << secondaryId << ")";

    {
        std::unique_lock<std::shared_mutex> lck{ secondariesMutex_ };
        auto                                it = secondaries_.find(secondaryId);
        if (it != secondaries_.end())
            it->second->status = SecondaryStatus::Active;
    }

    responsePromise->set_value(Proto::Response::SecondaryNodeReady{});
}

void MasterNode::removeSecondary(size_t secondaryId)
{
    EN_LOGI << "removeSecondary(secondaryId=" << secondaryId << ")";

    std::unique_lock<std::shared_mutex> lck{ secondariesMutex_ };
    auto                                it = secondaries_.find(secondaryId);
    if (it != secondaries_.end())
        secondaries_.erase(it);
}
