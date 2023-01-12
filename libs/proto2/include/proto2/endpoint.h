#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"
#include "utils/sharedpromise.h"
#include "utils/signal.h"
#include "utils/timestamp.h"

#include "proto2/duration.h"
#include "proto2/enums.h"

namespace Proto2
{
/**
 * Wraps outcoming requests into future/promise paradigm and
 * incoming requests into signal paradigm
 *
 * @note outcoming requests functions are thread safe, however
 * communication with the socket is done in a single thread
 *
 * @note run() does not prolong the lifetime of Endpoint object
 */
class Endpoint : public std::enable_shared_from_this<Endpoint>, private logger::StringIdEntity<Endpoint>
{
    DISABLE_COPY_MOVE(Endpoint)

public:
    [[nodiscard]] static std::shared_ptr<Endpoint> create(
        std::string                                                 id,
        boost::asio::io_context                                    &ioContext,
        boost::asio::ip::tcp::socket                                socket,
        duration_milliseconds_t                                     outcomingRequestTimeout,
        std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds);
    ~Endpoint();

    void run();

    Utils::signal<> invalidated;

public:    // outcoming requests (thread safe)
    boost::future<AddMessageStatus>                      send_addMessage(size_t msgId, std::string_view msg);
    boost::future<std::vector<std::string>>              send_getMessages(size_t startMsgId);
    boost::future<Utils::Timestamp_t /*pong timestamp*/> send_ping(Utils::Timestamp_t pingTimestamp);
    boost::future<void>                                  send_secondaryNodeReady(std::string secondaryName);

public:    // incoming requests (emitted in socket thread)
    // clang-format off
    Utils::signal<
        size_t                                  /*msgId*/,
        std::string                             /*msg*/,
        Utils::SharedPromise<AddMessageStatus>  /*responseStatus*/
    > incoming_addMessage;

    Utils::signal<
        size_t                                              /*startMsgId*/,
        Utils::SharedPromise<std::vector<std::string_view>> /*messages*/
    > incoming_getMessages;

    Utils::signal<
        Utils::Timestamp_t                          /*requestTimestamp*/,
        Utils::SharedPromise<Utils::Timestamp_t>    /*responseTimestamp*/
    > incoming_ping;

    Utils::signal<
        std::string                 /*secondaryNodeName*/,
        Utils::SharedPromise<void>  /*ack*/
    > incoming_secondaryNodeReady;
    // clang-format on

private:
    Endpoint(
        std::string,
        boost::asio::io_context &,
        boost::asio::ip::tcp::socket,
        duration_milliseconds_t                                     outcomingRequestTimeout,
        std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds);

    void establishConnections();

private:
    struct impl_t;

    impl_t       &impl();
    const impl_t &impl() const;

    const std::unique_ptr<impl_t> TW5VNznK_;
};

}    // namespace Proto2
