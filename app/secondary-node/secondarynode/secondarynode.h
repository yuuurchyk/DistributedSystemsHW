#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "proto/endpoint.h"
#include "utils/copymove.h"

#include "mastersession/mastersession.h"
#include "storage/storage.h"

class SecondaryNode : public std::enable_shared_from_this<SecondaryNode>, private logger::Entity<SecondaryNode>
{
    DISABLE_COPY_MOVE(SecondaryNode)
public:
    [[nodiscard]] static std::shared_ptr<SecondaryNode> create(
        std::string                               friendlyName,
        boost::asio::io_context                  &ioContext,
        boost::asio::ip::tcp::endpoint            masterAddress,
        std::chrono::duration<size_t, std::milli> masterReconnectInterval);
    ~SecondaryNode() = default;

    void run();

    // thread safe
    [[nodiscard]] bool operational() const;
    const Storage     &storage() const;

private:
    SecondaryNode(
        std::string,
        boost::asio::io_context &,
        boost::asio::ip::tcp::endpoint,
        std::chrono::duration<size_t, std::milli>);

    void reconnectToMaster();

    void disconnectMasterSession();
    void runMasterSession(boost::asio::ip::tcp::socket);

private:
    const std::string friendlyName_;

    Storage storage_;

    boost::asio::io_context              &ioContext_;
    const boost::asio::ip::tcp::endpoint  masterAddress_;
    std::shared_ptr<Proto::Endpoint>      masterEndpoint_;
    const boost::posix_time::milliseconds masterReconnectInterval_;

    mutable std::shared_mutex operationalMutex_;
    bool                      operational_{};

    std::shared_ptr<MasterSession> session_{};

    boost::signals2::scoped_connection invalidatedConnection_;
    boost::signals2::scoped_connection operationalConnection_;
};
