#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "proto2/endpoint.h"
#include "utils/copymove.h"

#include "secondarystorage/secondarystorage.h"

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

    // does not prolong the lifetime of SecondaryNode
    void run();

    // thread safe
    [[nodiscard]] bool      valid() const;
    const SecondaryStorage &storage() const;

private:
    SecondaryNode(
        std::string,
        boost::asio::io_context &,
        boost::asio::ip::tcp::endpoint,
        std::chrono::duration<size_t, std::milli>);

    void reconnectToMaster();

    void establishMasterEndpoint(boost::asio::ip::tcp::socket);

    void askMasterForMessages();
    void sendFriendlyNameToMaster();

    void invalidateMasterEndpoint();

private:
    const std::string friendlyName_;

    SecondaryStorage storage_;

    boost::asio::io_context              &ioContext_;
    const boost::asio::ip::tcp::endpoint  masterAddress_;
    std::shared_ptr<Proto2::Endpoint>     masterEndpoint_;
    const boost::posix_time::milliseconds masterReconnectInterval_;

    mutable std::shared_mutex validMutex_;
    bool                      valid_{};
};
