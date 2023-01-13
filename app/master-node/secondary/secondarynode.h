#pragma once

#include <memory>
#include <optional>
#include <string>

#include <boost/asio.hpp>

#include "net-utils/iocontextpool.h"
#include "proto/endpoint.h"
#include "utils/copymove.h"

#include "secondary/secondarystate.h"
#include "storage/storage.h"

class SecondaryNode
{
    DISABLE_COPY_MOVE(SecondaryNode)
public:
    [[nodiscard]] static std::shared_ptr<SecondaryNode> create(
        size_t                   id,
        boost::asio::io_context &ioContext,    // (related to socket)
        boost::asio::ip::tcp::socket,
        std::shared_ptr<NetUtils::IOContextPool::LoadGuard>);

    std::shared_ptr<Proto::Endpoint> endpoint();

    size_t                            id() const;
    const std::optional<std::string> &friendlyName() const;
    SecondaryState                    state() const;

    void setFriendlyName(std::string);
    void setOperational();
    void setInvalidated();

private:
    SecondaryNode(
        size_t id,
        boost::asio::io_context &,
        boost::asio::ip::tcp::socket,
        std::shared_ptr<NetUtils::IOContextPool::LoadGuard>);

    const size_t               id_;
    std::optional<std::string> friendlyName_;

    SecondaryState state_{ SecondaryState::REGISTERING };

    boost::asio::io_context               &ioContext_;
    const std::shared_ptr<Proto::Endpoint> endpoint_;

    const std::shared_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard_;
};
