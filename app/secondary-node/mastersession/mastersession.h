#pragma once

#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "proto/endpoint.h"
#include "proto/enums.h"
#include "utils/copymove.h"
#include "utils/sharedpromise.h"
#include "utils/signal.h"
#include "utils/timestamp.h"

#include "storage/storage.h"

class MasterSession : public std::enable_shared_from_this<MasterSession>, private logger::Entity<MasterSession>
{
    DISABLE_COPY_MOVE(MasterSession)
public:
    /**
     * @param friendlyName
     * @param ioContext - execution context, in which @p soket runs
     * @param socket
     * @param storage
     * @return std::shared_ptr<MasterSession>
     */
    [[nodiscard]] static std::shared_ptr<MasterSession> create(
        std::string                  friendlyName,
        boost::asio::io_context     &executionContext,
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

    void onEndpointInvalidated();
    void onIncomingAddMessage(size_t msgId, std::string msg, Utils::SharedPromise<Proto::AddMessageStatus> response);
    void onIncomingPing(Utils::Timestamp_t pingTimestamp, Utils::SharedPromise<Utils::Timestamp_t> response);

private:
    const std::string friendlyName_;

    boost::asio::io_context               &executionContext_;
    const std::shared_ptr<Proto::Endpoint> endpoint_;

    Storage &storage_;
};
