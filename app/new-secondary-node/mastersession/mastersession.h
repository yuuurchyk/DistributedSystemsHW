#pragma once

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "proto2/endpoint.h"
#include "utils/copymove.h"

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

    boost::signals2::signal<void()> invalidated;
    boost::signals2::signal<void()> operational;

private:
    MasterSession(std::string, boost::asio::io_context &, boost::asio::ip::tcp::socket, Storage &);

    void establishConnections();

    void askForMessages();
    void notifyOperational();

private:
    const std::string friendlyName_;

    boost::asio::io_context                &ioContext_;
    const std::shared_ptr<Proto2::Endpoint> endpoint_;

    Storage &storage_;
};
