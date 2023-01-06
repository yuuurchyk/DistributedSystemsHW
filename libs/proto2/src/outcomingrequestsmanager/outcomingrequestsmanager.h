// #pragma once

// #include <atomic>
// #include <memory>
// #include <queue>
// #include <unordered_map>

// #include <boost/asio.hpp>
// #include <boost/signals2.hpp>

// #include "logger/entity.hpp"

// #include "utils/copymove.h"

// #include "outcomingrequestcontext/abstractoutcomingrequestcontext.h"
// #include "request/abstractrequest.h"

// namespace Proto2
// {
// class OutcomingRequestsManager : private logger::StringIdEntity<OutcomingRequestsManager>
// {
//     DISABLE_COPY_MOVE(OutcomingRequestsManager)
// public:
//     [[nodiscard]] std::unique_ptr<OutcomingRequestsManager>
//         create(std::shared_ptr<boost::asio::ip::tcp::socket>, size_t responseTimeoutMs);

//     boost::signals2::signal<void()> invalidated;

//     // thread safe
//     void send(
//         std::shared_ptr<Request::AbstractRequest>,
//         std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext>);

// private:
//     OutcomingRequestsManager(std::shared_ptr<boost::asio::ip::tcp::socket>, size_t responseTimeoutMs);

//     void invalidate();

//     size_t getNextRequestId();

// private:
//     bool                                                invalidated_{};
//     const std::shared_ptr<boost::asio::ip::tcp::socket> socket_;

//     std::atomic<size_t> requestIdCounter_;
//     size_t              requestIdBuffer_;

//     const size_t responseTimeoutMs_;

//     struct Pending
//     {
//         DISABLE_COPY_DEFAULT_MOVE(Pending)

//         size_t                                                                    id;
//         boost::asio::deadline_timer                                               timeoutTimer;
//         std::shared_ptr<Request::AbstractRequest>                                 request;
//         std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext> context;
//     };

//     std::unordered_map<size_t /* requestId */, Pending> requests_;
// };

// }    // namespace Proto2
