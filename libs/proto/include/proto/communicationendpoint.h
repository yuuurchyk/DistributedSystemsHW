// #pragma once

// #include <atomic>
// #include <memory>
// #include <optional>
// #include <queue>
// #include <string>
// #include <unordered_set>
// #include <vector>

// #include <boost/asio.hpp>
// #include <boost/signals2.hpp>
// #include <boost/thread.hpp>

// #include "logger/logger.h"
// #include "proto/proto.h"
// #include "utils/copymove.h"

// namespace Proto
// {
// /**
//  * @brief websocket implementation of messages exchange system
//  *
//  */
// class CommunicationEndpoint : public
// std::enable_shared_from_this<CommunicationEndpoint>,
//                               public logger::StringIdEntity<CommunicationEndpoint>
// {
//     DISABLE_COPY_MOVE(CommunicationEndpoint)
// public:
//     [[nodiscard]] static std::shared_ptr<CommunicationEndpoint>
//         create(boost::asio::io_context &, boost::asio::ip::tcp::socket);

//     ~CommunicationEndpoint();

//     // thread safe
//     boost::future<Response::AddMessage>
//         addMessage(std::shared_ptr<const Request::AddMessage>,
//                    std::optional<size_t> timeoutMs);
//     boost::future<Response::GetMessages>
//         getMessages(std::shared_ptr<const Request::GetMessages>,
//                     std::optional<size_t> timeoutMs);

// public:    // signals
//     // clang-format off
//     template <typename T>
//     using signal = boost::signals2::signal<T>;

//     signal<void()> invalidated;

//     signal<void(Request::AddMessage  &,
//     std::shared_ptr<boost::promise<Response::AddMessage>>)>   requestAddMessages;
//     signal<void(Request::GetMessages &,
//     std::shared_ptr<boost::promise<Response::GetMessages>>)> requestGetMessages;
//     // clang-format on

// private:
//     RequestId_t getNextRequestId();

// private:
//     boost::asio::io_context     &io_context_;
//     boost::asio::ip::tcp::socket socket_;

//     std::atomic<RequestId_t> requestIdCounter_;

//     class PendingRequestContext;
//     std::unordered_set<std::unique_ptr<PendingRequestContext>> pendingRequests_;
// };

// }    // namespace Proto
