#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "proto2/endpoint.h"
#include "proto2/sharedpromise.h"
#include "proto2/timestamp.h"
#include "utils/copymove.h"

#include "secondary/secondarynode.h"
#include "secondary/secondarysnapshot.h"
#include "storage/storage.h"

class MasterNode : public std::enable_shared_from_this<MasterNode>, private logger::Entity<MasterNode>
{
public:    // usage interface
    [[nodiscard]] std::shared_ptr<MasterNode> create(boost::asio::io_context &);

    void addSecondary(
        boost::asio::io_context &secondaryContext,
        boost::asio::ip::tcp::socket,
        std::unique_ptr<NetUtils::IOContextPool::LoadGuard>);

    const Storage &storage() const;

public:
    boost::future<void> addMessage(boost::asio::io_context &executionContext, std::string message, size_t writeConcern);

    std::vector<SecondarySnapshot> secondariesSnapshot();

private:
    MasterNode(boost::asio::io_context &);

    void addSecondaryImpl(
        boost::asio::io_context &,
        boost::asio::ip::tcp::socket,
        std::shared_ptr<NetUtils::IOContextPool::LoadGuard>);

    void onInvalidated(size_t secondaryId);
    void onIncomingGetMessages(size_t startMsgId, Proto2::SharedPromise<std::vector<std::string_view>> response);
    void onIncomingSecondaryNodeReady(
        size_t                      secondaryId,
        std::string                 friendlyName,
        Proto2::SharedPromise<void> response);

private:
    boost::asio::io_context &ioContext_;
    size_t                   secondaryIdCounter_{};

    std::shared_mutex                                          secondariesMutex_;
    std::unordered_map<size_t, std::shared_ptr<SecondaryNode>> secondaries_;

    Storage storage_;
};
