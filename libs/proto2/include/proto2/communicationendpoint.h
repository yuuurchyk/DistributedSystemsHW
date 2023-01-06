// #pragma once

// #include <chrono>
// #include <memory>
// #include <string>
// #include <string_view>
// #include <vector>

// #include <boost/asio.hpp>
// #include <boost/signals2.hpp>
// #include <boost/thread.hpp>

// #include "utils/copymove.h"

// #include "proto2/enums.h"
// #include "proto2/timestamp.h"

// namespace Proto2
// {

// /**
//  * Wraps outcoming requests into future/promise paradigm and
//  * incoming requests into signals paradigm
//  *
//  * @note outcoming requests functions are thread safe, however
//  * communication with the socket is done in a single thread
//  */
// class CommunicationEndpoint
// {
//     DISABLE_COPY_MOVE(CommunicationEndpoint)

// public:
//     [[nodiscard]] static std::unique_ptr<CommunicationEndpoint> create(
//         boost::asio::io_context     &ioContext,
//         boost::asio::ip::tcp::socket socket,
//         std::chrono::milliseconds    outcomingRequestTimeout);
//     ~CommunicationEndpoint();

//     boost::signals2::signal<void()> invalidated;

// public:    // outcoming requests
//     boost::future<AddMessageStatus>         addMessage(size_t msgId, std::string_view msg);
//     boost::future<std::vector<std::string>> getMessages(size_t startMsgId);
//     boost::future<Timestamp_t>              ping(Timestamp_t pingTimestamp);
//     boost::future<void>                     secondaryNodeReady(std::string secondaryName);

// public:    // incoming requests
//     template <typename... Args>
//     using signal = boost::signals2::signal<void(Args...)>;
//     template <typename T>
//     using SharedPromise = std::shared_ptr<boost::promise<T>>;

//     // clang-format off
//     signal<
//         size_t                          /*msgId*/,
//         std::string_view                /*msg*/,
//         SharedPromise<AddMessageStatus> /*responseStatus*/
//     > incoming_addMessage;

//     signal<
//         size_t                                          /*startMsgId*/,
//         SharedPromise<std::vector<std::string_view>>    /*messages*/
//     > incoming_getMessages;

//     signal<
//         Timestamp_t                 /*requestTimestamp*/,
//         SharedPromise<Timestamp_t>  /*responseTimestamp*/
//     > incoming_ping;

//     signal<
//         std::string         /*secondaryNodeName*/,
//         SharedPromise<void> /*ack*/
//     > incoming_secondaryNodeReady;
//     // clang-format on

// private:
//     CommunicationEndpoint(boost::asio::io_context &, boost::asio::ip::tcp::socket);

// private:
//     struct impl_t;

//     impl_t       &impl();
//     const impl_t &impl() const;

//     std::unique_ptr<impl_t> TW5VNznK_;
// };

// }    // namespace Proto2
