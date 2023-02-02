#include "masternode/masternode.h"

#include "addmessagerequest/addmessagerequest.h"
#include "net-utils/thenpost.h"

std::shared_ptr<MasterNode> MasterNode::create(boost::asio::io_context &context)
{
    return std::shared_ptr<MasterNode>{ new MasterNode{ context } };
}

boost::future<void>
    MasterNode::addMessage(boost::asio::io_context &executionContext, std::string message, size_t writeConcern)
{
    auto [msgId, msgView] = storage_.addMessage(std::move(message));

    auto request = AddMessageRequest::create(
        executionContext, weak_from_this(), msgId, msgView, writeConcern <= 1 ? size_t{} : writeConcern - 1);

    auto requestFuture = request->future();
    request->run();

    if (writeConcern <= 1)
    {
        // no confirmations need from secondaries, returning ready future
        auto promise = boost::promise<void>{};
        promise.set_value();
        return promise.get_future();
    }
    else
    {
        return requestFuture;
    }
}

boost::future<std::vector<MasterNode::PingResult>>
    MasterNode::pingSecondaries(boost::asio::io_context &executionContext)
{
    const auto pingTimestamp = Utils::getCurrentTimestamp();

    auto snapshot = secondariesSnapshot();

    if (snapshot.empty())
    {
        auto promise = boost::promise<std::vector<MasterNode::PingResult>>{};
        promise.set_value({});
        auto future = promise.get_future();
        return future;
    }

    auto responses = std::vector<boost::future<Utils::Timestamp_t>>{};
    responses.reserve(snapshot.size());

    for (auto &secondary : snapshot)
        responses.push_back(secondary.endpoint->send_ping(pingTimestamp));

    auto promise = Utils::makeSharedPromise(boost::promise<std::vector<PingResult>>{});
    auto future  = promise->get_future();

    NetUtils::thenPost(
        boost::when_all(responses.begin(), responses.end()),
        executionContext,
        [pingTimestamp, snapshot = std::move(snapshot), promise](
            boost::future<std::vector<boost::future<Utils::Timestamp_t>>> responsesFuture) mutable
        {
            auto responses = std::vector<boost::future<Utils::Timestamp_t>>{};
            try
            {
                responses = responsesFuture.get();
            }
            catch (...)
            {
                promise->set_exception(boost::current_exception());
                return;
            }

            auto pingResults = std::vector<PingResult>{};
            pingResults.reserve(responses.size());

            for (auto i = size_t{}; i < responses.size(); ++i)
            {
                auto       &responseFuture = responses.at(i);
                const auto &secondary      = snapshot.at(i);

                auto pingResult = PingResult{};

                pingResult.secondaryId           = secondary.id;
                pingResult.secondaryFriendlyName = secondary.friendlyName;
                pingResult.secondaryState        = secondary.state;

                pingResult.pingTimestamp = pingTimestamp;

                try
                {
                    pingResult.pongTimestamp = responseFuture.get();
                }
                catch (const boost::exception &e)
                {
                    pingResult.exceptionString = boost::diagnostic_information(e);
                }
                catch (const std::exception &e)
                {
                    pingResult.exceptionString = boost::diagnostic_information(e);
                }

                pingResults.push_back(std::move(pingResult));
            }

            promise->set_value(std::move(pingResults));
        });

    return future;
}

void MasterNode::addSecondary(
    boost::asio::io_context                            &secondaryContext,
    boost::asio::ip::tcp::socket                        socket,
    std::unique_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard)
{
    auto sharedLoadGuard = std::shared_ptr<NetUtils::IOContextPool::LoadGuard>{ loadGuard.release() };

    boost::asio::post(
        ioContext_,
        [this,
         &secondaryContext,
         socket    = std::move(socket),
         loadGuard = std::move(sharedLoadGuard),
         weakSelf  = weak_from_this()]() mutable
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            addSecondaryImpl(secondaryContext, std::move(socket), std::move(loadGuard));
        });
}

void MasterNode::addSecondaryImpl(
    boost::asio::io_context                            &secondaryContext,
    boost::asio::ip::tcp::socket                        socket,
    std::shared_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard)
{
    const std::unique_lock lck{ secondariesMutex_ };
    const auto             id = secondaryIdCounter_++;
    auto secondary            = SecondaryNode::create(id, secondaryContext, std::move(socket), std::move(loadGuard));
    secondaries_[id]          = secondary;

    EN_LOGI << "adding secondary: id=" << id;

    auto &masterNodeContext = ioContext_;

    auto endpoint = secondary->endpoint();

    // endpoint might be running in different thread, so make sure to execute slots
    // in the right io context

    endpoint->invalidated.connect(
        [this, &masterNodeContext, id, weakSelf = weak_from_this()]()
        {
            boost::asio::post(
                masterNodeContext,
                [this, id, weakSelf]()
                {
                    const auto self = weakSelf.lock();
                    if (self != nullptr)
                        onInvalidated(id);
                });
        });

    endpoint->incoming_getMessages.connect(
        [this, &masterNodeContext, weakSelf = weak_from_this()](
            size_t startMsgId, Utils::SharedPromise<std::vector<std::string_view>> response) mutable
        {
            boost::asio::post(
                masterNodeContext,
                [this, startMsgId, response = std::move(response), weakSelf]() mutable
                {
                    const auto self = weakSelf.lock();
                    if (self != nullptr)
                        onIncomingGetMessages(startMsgId, std::move(response));
                });
        });
    endpoint->incoming_secondaryNodeReady.connect(
        [this, &masterNodeContext, id, weakSelf = weak_from_this()](
            std::string friendlyName, Utils::SharedPromise<void> response) mutable
        {
            boost::asio::post(
                masterNodeContext,
                [this, id, friendlyName = std::move(friendlyName), response = std::move(response), weakSelf]() mutable
                {
                    const auto self = weakSelf.lock();
                    if (self != nullptr)
                        onIncomingSecondaryNodeReady(id, std::move(friendlyName), std::move(response));
                });
        });

    endpoint->run();
}

void MasterNode::onInvalidated(size_t secondaryId)
{
    const auto it = secondaries_.find(secondaryId);
    if (it == secondaries_.end())
        return;

    EN_LOGW << "secondary " << secondaryId << " invalidated";

    const std::unique_lock lck{ secondariesMutex_ };
    it->second->setInvalidated();
    secondaries_.erase(secondaryId);
}

void MasterNode::onIncomingGetMessages(size_t startMsgId, Utils::SharedPromise<std::vector<std::string_view>> response)
{
    response->set_value(storage().getContiguousChunk(startMsgId));
}

void MasterNode::onIncomingSecondaryNodeReady(
    size_t                     secondaryId,
    std::string                friendlyName,
    Utils::SharedPromise<void> response)
{
    const auto it = secondaries_.find(secondaryId);
    if (it == secondaries_.end())
        return;

    EN_LOGI << "registering node " << secondaryId << " as ready, friendlyName: " << friendlyName;

    {
        const std::unique_lock lck{ secondariesMutex_ };
        it->second->setFriendlyName(std::move(friendlyName));
        it->second->setOperational();
    }

    response->set_value();
}

std::vector<SecondarySnapshot> MasterNode::secondariesSnapshot()
{
    auto                   res = std::vector<SecondarySnapshot>{};
    const std::shared_lock lck{ secondariesMutex_ };
    res.reserve(secondaries_.size());

    for (auto &[id, secondary] : secondaries_)
    {
        auto snapshot = SecondarySnapshot{};

        snapshot.id           = id;
        snapshot.state        = secondary->state();
        snapshot.friendlyName = secondary->friendlyName();
        snapshot.endpoint     = secondary->endpoint();

        res.push_back(std::move(snapshot));
    }

    return res;
}

MasterNode::MasterNode(boost::asio::io_context &ioContext) : ioContext_{ ioContext } {}

const Storage &MasterNode::storage() const
{
    return storage_;
}
