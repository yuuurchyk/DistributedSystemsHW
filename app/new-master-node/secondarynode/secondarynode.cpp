#include "secondarynode/secondarynode.h"

#include <utility>

#include "constants2/constants2.h"

std::shared_ptr<SecondaryNode> SecondaryNode::create(
    size_t                                              id,
    boost::asio::io_context                            &ioContext,
    boost::asio::ip::tcp::socket                        socket,
    std::shared_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard)
{
    return std::shared_ptr<SecondaryNode>{ new SecondaryNode{
        id, ioContext, std::move(socket), std::move(loadGuard) } };
}

std::shared_ptr<Proto2::Endpoint> SecondaryNode::endpoint()
{
    return endpoint_;
}

size_t SecondaryNode::id() const
{
    return id_;
}

const std::optional<std::string> &SecondaryNode::friendlyName() const
{
    return friendlyName_;
}

SecondaryNode::State SecondaryNode::state() const
{
    return state_;
}

void SecondaryNode::setFriendlyName(std::string name)
{
    friendlyName_ = std::move(name);
}

void SecondaryNode::setOperational()
{
    state_ = State::OPERATIONAL;
}

void SecondaryNode::setInvalidated()
{
    state_ = State::INVALIDATED;
}

SecondaryNode::SecondaryNode(
    size_t                                              id,
    boost::asio::io_context                            &ioContext,
    boost::asio::ip::tcp::socket                        socket,
    std::shared_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard)
    : id_{ id },
      ioContext_{ ioContext },
      endpoint_{ Proto2::Endpoint::create(
          "sec_" + std::to_string(id),
          ioContext,
          std::move(socket),
          Constants2::kOutcomingRequestTimeout,
          Constants2::kArtificialSendDelayBounds) },
      loadGuard_{ std::move(loadGuard) }
{
}
