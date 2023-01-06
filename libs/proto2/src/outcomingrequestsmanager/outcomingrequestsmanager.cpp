// #include "outcomingrequestsmanager.h"

// #include <utility>

// namespace Proto2
// {

// std::unique_ptr<OutcomingRequestsManager>
//     OutcomingRequestsManager::create(std::shared_ptr<boost::asio::ip::tcp::socket> socket, size_t responseTimeoutMs)
// {
//     return std::unique_ptr<OutcomingRequestsManager>{ new OutcomingRequestsManager{ std::move(socket),
//                                                                                     responseTimeoutMs } };
// }

// void OutcomingRequestsManager::send(
//     std::shared_ptr<Request::AbstractRequest>                                 request,
//     std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext> requestContext)
// {
//     if (request == nullptr)
//     {
//         EN_LOGW << "request should not be nullptr, not sending request";
//         return;
//     }
//     if (requestContext == nullptr)
//     {
//         EN_LOGW << "requestContext should not be nullptr, not sending request";
//         return;
//     }

//     const auto requestId = getNextRequestId();
// }

// }    // namespace Proto2
