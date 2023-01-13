#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "proto/endpoint.h"
#include "utils/copymove.h"
#include "utils/signal.h"

#include "storage/storage.h"

class MasterSession : public std::enable_shared_from_this<MasterSession>, private logger::Entity<MasterSession>
{
    DISABLE_COPY_MOVE(MasterSession)
public:
    [[nodiscard]] static std::shared_ptr<MasterSession> create(
        std::string                  friendlyName,
        boost::asio::io_context     &ioContext,
        boost::asio::ip::tcp::socket socket,
        Storage                     &storage);

    void run();

    Utils::signal<> invalidated;
    Utils::signal<> operational;

private:
    MasterSession(std::string, boost::asio::io_context &, boost::asio::ip::tcp::socket, Storage &);

    void establishConnections();

    void askForMessages();
    void notifyOperational();

private:
    const std::string friendlyName_;

    boost::asio::io_context               &ioContext_;
    const std::shared_ptr<Proto::Endpoint> endpoint_;

    Storage &storage_;

    std::vector<boost::signals2::scoped_connection> connections_;
};
