#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "proto2/endpoint.h"
#include "utils/copymove.h"

#include "storage/storage.h"

class MasterNode : public std::enable_shared_from_this<MasterNode>, private logger::Entity<MasterNode>
{
public:    // usage interface
    [[nodiscard]] std::shared_ptr<MasterNode> create(boost::asio::io_context &);

    void addSecondary(boost::asio::ip::tcp::socket, std::unique_ptr<NetUtils::IOContextPool::LoadGuard>);
    void run();

    const Storage &storage() const;

    boost::future<void> addMessage(std::string message, size_t writeConcern);

public:    // interface for composite requests
    enum class SecondaryStatus
    {
        REGISTERING,
        ACTIVE
    };

    struct SecondaryInfo
    {
        SecondaryStatus                   status;
        std::shared_ptr<Proto2::Endpoint> endpoint;
    };

    std::vector<SecondaryInfo> secondariesSnapshot();

private:
    struct Secondary
    {
        DISABLE_COPY_MOVE(Secondary)

        Secondary(boost::asio::ip::tcp::socket, std::unique_ptr<NetUtils::IOContextPool::LoadGuard>);

        size_t      id;
        std::string friendlyName;

        SecondaryStatus                                     status{ SecondaryStatus::REGISTERING };
        std::shared_ptr<Endpoint>                           endpoint;
        std::unique_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard;

        std::vector<boost::signals2::connection> connections;
    };

    boost::asio::io_context &ioContext_;
    size_t                   secondaryIdCounter_{};

    std::unordered_map<size_t, std::unique_ptr<Secondary>> secondaries_;

    Storage storage_;
};
