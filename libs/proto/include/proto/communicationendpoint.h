// #pragma once

// #include <atomic>
// #include <optional>
// #include <unordered_map>

// #include <boost/asio.hpp>
// #include <boost/signals2.hpp>
// #include <boost/thread.hpp>

// #include "logger/logger.h"
// #include "proto/concepts.h"
// #include "proto/proto.h"
// #include "proto/socketwrapper.h"
// #include "reflection/deserialization.h"
// #include "utils/copymove.h"

// namespace Proto
// {
// // express requests/repsonse in terms of futures and signals
// class CommunicationEndpoint : public std::enable_shared_from_this<CommunicationEndpoint>,
//                               public logger::Entity<CommunicationEndpoint>
// {
//     DISABLE_COPY_MOVE(CommunicationEndpoint)
// public:
//     template <typename T>
//     using signal = boost::signals2::signal<T>;
//     template <Concepts::Request Request>
//     using RequestSignal =
//         signal<void(std::shared_ptr<const Request>,
//                     std::shared_ptr<boost::promise<Concepts::Response_t<Request>>>)>;

//     [[nodiscard]] static std::shared_ptr<CommunicationEndpoint>
//         create(boost::asio::ip::tcp::socket socket, size_t sendTimeoutMs = 3000);

//     ~CommunicationEndpoint();

//     void run();

//     // thread safe
//     boost::future<Response::AddMessage>
//         send_addMessage(std::shared_ptr<const Request::AddMessage>);
//     boost::future<Response::GetMessages>
//         send_getMessages(std::shared_ptr<const Request::GetMessages>);

//     RequestSignal<Request::AddMessage>   incoming_addMessage;
//     RequestSignal<Requeset::GetMessages> incoming_getMessages;

//     signal<void()> invalidated;

// private:
//     CommunicationEndpoint(boost::asio::ip::tcp::socket, size_t sendTimeoutMs);

//     template <Concepts::Request Request>
//     boost::future<Concepts::Response_t<Request>>
//         sendRequestImpl(std::shared_ptr<const Request>);
//     template <Concepts::Response Response>
//     void sendResponseImpl(size_t requestId, std::shared_ptr<const Response>);

//     void onIncomingBuffer(boost::asio::const_buffer);
//     void onIncomingResponse(
//         size_t                                                         requestId,
//         Reflection::DeserializationContext<boost::asio::const_buffer> &body);
//     void onIncomingRequest(
//         size_t                                                         requestId,
//         OpCode                                                         opCode,
//         Reflection::DeserializationContext<boost::asio::const_buffer> &body);

//     template <>

//     void invalidate();

//     size_t getNextRequestId();

//     // request that is waiting for response from the other side
//     struct AbstractPendingRequest;
//     // context from incoming request
//     struct AbstractPendingResponse;

// private:
//     const size_t                   sendTimeoutMs_;
//     boost::asio::any_io_executor   executor_;
//     std::shared_ptr<SocketWrapper> socket_;
//     std::atomic<size_t>            requestCounter_{};

//     std::unordered_map<size_t, std::shared_ptr<AbstractPendingRequest>> pendingRequests_;
//     std::unordered_map<size_t, std::shared_ptr<boost::asio::deadline_timer>>
//         pendingRequestsTimers_;

//     std::unordered_map<size_t, std::shared_ptr<AbstractPendingResponse>>
//         pendingResponses_;
// };

// }    // namespace Proto

// #include "detail/communicationendpoint_impl.h"

// // #include <atomic>
// // #include <memory>
// // #include <optional>
// // #include <queue>
// // #include <string>
// // #include <unordered_set>
// // #include <vector>

// // #include <boost/asio.hpp>
// // #include <boost/signals2.hpp>
// // #include <boost/thread.hpp>

// // #include "logger/logger.h"
// // #include "proto/proto.h"
// // #include "utils/copymove.h"

// // namespace Proto
// // {
// // /**
// //  * @brief websocket implementation of messages exchange system
// //  *
// //  */
// // class CommunicationEndpoint : public
// // std::enable_shared_from_this<CommunicationEndpoint>,
// //                               public logger::StringIdEntity<CommunicationEndpoint>
// // {
// //     DISABLE_COPY_MOVE(CommunicationEndpoint)
// // public:
// //     [[nodiscard]] static std::shared_ptr<CommunicationEndpoint>
// //         create(boost::asio::io_context &, boost::asio::ip::tcp::socket);

// //     ~CommunicationEndpoint();

// //     // thread safe
// //     boost::future<Response::AddMessage>
// //         addMessage(std::shared_ptr<const Request::AddMessage>,
// //                    std::optional<size_t> timeoutMs);
// //     boost::future<Response::GetMessages>
// //         getMessages(std::shared_ptr<const Request::GetMessages>,
// //                     std::optional<size_t> timeoutMs);

// // public:    // signals
// //     // clang-format off
// //     template <typename T>
// //     using signal = boost::signals2::signal<T>;

// //     signal<void()> invalidated;

// //     signal<void(Request::AddMessage  &,
// //     std::shared_ptr<boost::promise<Response::AddMessage>>)>   requestAddMessages;
// //     signal<void(Request::GetMessages &,
// //     std::shared_ptr<boost::promise<Response::GetMessages>>)> requestGetMessages;
// //     // clang-format on

// // private:
// //     RequestId_t getNextRequestId();

// // private:
// //     boost::asio::io_context     &io_context_;
// //     boost::asio::ip::tcp::socket socket_;

// //     std::atomic<RequestId_t> requestIdCounter_;

// //     class PendingRequestContext;
// //     std::unordered_set<std::unique_ptr<PendingRequestContext>> pendingRequests_;
// // };

// // }    // namespace Proto
