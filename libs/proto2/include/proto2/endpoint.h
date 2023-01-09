#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "proto2/duration.h"
#include "proto2/enums.h"
#include "proto2/sharedpromise.h"
#include "proto2/signal.h"
#include "proto2/timestamp.h"

namespace Proto2
{
/**
 * Wraps outcoming requests into future/promise paradigm and
 * incoming requests into signal paradigm
 *
 * @note outcoming requests functions are thread safe, however
 * communication with the socket is done in a single thread
 */
class Endpoint : public std::enable_shared_from_this<Endpoint>, private logger::NumIdEntity<Endpoint>
{
    DISABLE_COPY_MOVE(Endpoint)

public:
    [[nodiscard]] static std::shared_ptr<Endpoint> create(
        boost::asio::io_context                                    &ioContext,
        boost::asio::ip::tcp::socket                                socket,
        duration_milliseconds_t                                     outcomingRequestTimeout,
        std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds);
    ~Endpoint();

    void run();

    signal<> invalidated;

public:    // outcoming requests (thread safe)
    boost::future<AddMessageStatus>               send_addMessage(size_t msgId, std::string_view msg);
    boost::future<std::vector<std::string>>       send_getMessages(size_t startMsgId);
    boost::future<Timestamp_t /*pong timestamp*/> send_ping(Timestamp_t pingTimestamp);
    boost::future<void>                           send_secondaryNodeReady(std::string secondaryName);

public:    // incoming requests (emitted in socket thread)
    // clang-format off
    signal<
        size_t                          /*msgId*/,
        std::string                     /*msg*/,
        SharedPromise<AddMessageStatus> /*responseStatus*/
    > incoming_addMessage;

    signal<
        size_t                                          /*startMsgId*/,
        SharedPromise<std::vector<std::string_view>>    /*messages*/
    > incoming_getMessages;

    signal<
        Timestamp_t                 /*requestTimestamp*/,
        SharedPromise<Timestamp_t>  /*responseTimestamp*/
    > incoming_ping;

    signal<
        std::string         /*secondaryNodeName*/,
        SharedPromise<void> /*ack*/
    > incoming_secondaryNodeReady;
    // clang-format on

private:
    Endpoint(
        boost::asio::io_context &,
        boost::asio::ip::tcp::socket,
        duration_milliseconds_t                                     outcomingRequestTimeout,
        std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds);

    void establishConnections();

private:
    struct impl_t;

    impl_t       &impl();
    const impl_t &impl() const;

    std::unique_ptr<impl_t> TW5VNznK_;
};

}    // namespace Proto2
