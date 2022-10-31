#include "secondariespool.h"

#include <stdexcept>
#include <utility>

#include "constants/constants.h"
#include "logger/logger.h"

std::shared_ptr<SecondariesPool>
    SecondariesPool::create(boost::asio::io_context                    &context,
                            std::vector<boost::asio::ip::tcp::endpoint> endpoints,
                            IOContextPool &secondariesContexts)
{
    return std::shared_ptr<SecondariesPool>{ new SecondariesPool{
        context, std::move(endpoints), secondariesContexts } };
}

void SecondariesPool::sendRequest(protocol::request::Request request,
                                  response_callback_fn       responseCallback)
{
    if (responseCallback == nullptr)
        responseCallback = [](std::vector<std::optional<protocol::response::Response>>,
                              protocol::request::Request) {};

    auto pendingRequest = PendingRequest::create(request, std::move(responseCallback));

    for (auto &secondary : secondaries_)
    {
        auto r = request;
        r.setRequestId(secondary->getNextRequestId());
        secondary->sendRequest(request,
                               [this, self = shared_from_this(), pendingRequest](
                                   std::optional<protocol::response::Response> response,
                                   protocol::request::Request)
                               {
                                   context_.post(
                                       [this,
                                        self           = shared_from_this(),
                                        pendingRequest = std::move(pendingRequest),
                                        response       = std::move(response)]() {
                                           handleResponseFromSecondary(
                                               std::move(pendingRequest),
                                               std::move(response));
                                       });
                               });
    }
}

SecondariesPool::SecondariesPool(boost::asio::io_context                    &context,
                                 std::vector<boost::asio::ip::tcp::endpoint> endpoints,
                                 IOContextPool                              &pool)
    : context_{ context }
{
    if (endpoints.empty())
        throw std::runtime_error{ "0 secondary nodes provided in secondaries pool" };

    for (const auto &endpoint : endpoints)
    {
        secondaries_.push_back(SecondarySession::create(
            pool.getNext(),
            endpoint,
            boost::posix_time::milliseconds{ constants::kSecondaryReconnectTimeoutMs }));
        secondaries_.back()->run();
    }
}

void SecondariesPool::handleResponseFromSecondary(
    std::shared_ptr<PendingRequest>             pendingRequest,
    std::optional<protocol::response::Response> response)
{
    LOGI << "response got from secondary, previous responses="
         << pendingRequest->responsesRecieved;

    ++pendingRequest->responsesRecieved;
    pendingRequest->responses.push_back(std::move(response));

    if (pendingRequest->responsesRecieved == secondaries_.size())
    {
        LOGI << "all secondaries responded, ecuting callback";
        pendingRequest->responseCallback(std::move(pendingRequest->responses),
                                         pendingRequest->request);
    }
}

auto SecondariesPool::PendingRequest::create(protocol::request::Request request,
                                             response_callback_fn       responseCallback)
    -> std::shared_ptr<PendingRequest>
{
    return std::shared_ptr<PendingRequest>{ new PendingRequest{
        std::move(request), std::move(responseCallback) } };
}

SecondariesPool::PendingRequest::PendingRequest(protocol::request::Request request,
                                                response_callback_fn       callback)
    : request{ std::move(request) }, responseCallback{ std::move(callback) }
{
}
