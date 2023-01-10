#include "masternode/masternode.h"

std::shared_ptr<MasterNode> MasterNode::create(boost::asio::io_context &context)
{
    return std::shared_ptr<MasterNode>{ new MasterNode{ context } };
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
    const auto id        = secondaryIdCounter_++;
    auto       secondary = SecondaryNode::create(id, secondaryContext, std::move(socket), std::move(loadGuard));
    secondaries_[id]     = secondary;

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
            size_t startMsgId, Proto2::SharedPromise<std::vector<std::string_view>> response) mutable
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
            std::string friendlyName, Proto2::SharedPromise<void> response) mutable
        {
            boost::asio::post(
                masterNodeContext,
                [this, id, friendlyName = std::move(friendlyName), response = std::move(response), weakSelf]() mutable
                {
                    const auto self = weakSelf.lock();
                    if (self == nullptr)
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

    it->second->setInvalidated();
    secondaries_.erase(secondaryId);
}

void MasterNode::onIncomingGetMessages(size_t startMsgId, Proto2::SharedPromise<std::vector<std::string_view>> response)
{
    response->set_value(storage().getContiguousChunk(startMsgId));
}

void MasterNode::onIncomingSecondaryNodeReady(
    size_t                      secondaryId,
    std::string                 friendlyName,
    Proto2::SharedPromise<void> response)
{
    const auto it = secondaries_.find(secondaryId);
    if (it == secondaries_.end())
        return;

    EN_LOGI << "registering node " << secondaryId << " as ready, friendlyName: " << friendlyName;

    it->second->setFriendlyName(std::move(friendlyName));
    it->second->setOperational();

    response->set_value();
}

MasterNode::MasterNode(boost::asio::io_context &ioContext) : ioContext_{ ioContext } {}

const Storage &MasterNode::storage() const
{
    return storage_;
}
