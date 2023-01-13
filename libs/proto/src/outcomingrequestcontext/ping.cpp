#include "outcomingrequestcontext/ping.h"

#include "response/pong.h"

namespace Proto::OutcomingRequestContext
{
std::shared_ptr<Ping> Ping::create()
{
    return std::shared_ptr<Ping>{ new Ping{} };
}

boost::future<Utils::Timestamp_t> Ping::get_future()
{
    return promise_.get_future();
}

void Ping::onResponseRecieved(boost::asio::const_buffer payload)
{
    if (promiseFilled())
        return;

    auto response = Response::Pong::fromPayload(payload);
    if (response == nullptr)
    {
        invalidate(InvalidationReason::BAD_FRAME);
    }
    else
    {
        promiseMarkFilled();
        promise_.set_value(response->timestamp());
    }
}

void Ping::invalidate(InvalidationReason reason)
{
    invalidatePromise(promise_, reason);
}

}    // namespace Proto::OutcomingRequestContext
