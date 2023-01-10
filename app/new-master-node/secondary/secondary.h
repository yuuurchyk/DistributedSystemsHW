#pragma once

#include <memory>
#include <string>

#include "net-utils/iocontextpool.h"
#include "utils/copymove.h"

#include "storage/storage.h"

class SecondaryNode
{
    DISABLE_COPY_MOVE(SecondaryNode)
public:
    [[nodiscard]] std::shared_ptr<SecondaryNode>
        create(size_t id, boost::asio::ip::tcp::socket, std::shared_ptr<NetUtils::IOContextPool::LoadGuard>);

private:
    SecondaryNode(size_t id, boost::asio::ip::tcp::socket, std::shared_ptr<NetUtils::IOContextPool::LoadGuard>);

    const size_t id_;

    const std::shared_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard_;
};
