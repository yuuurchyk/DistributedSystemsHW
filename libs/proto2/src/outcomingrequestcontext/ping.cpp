#include "proto2/outcomingrequestcontext/ping.h"

#include "response/pong.h"

namespace Proto2::OutcomingRequestContext
{

std::shared_ptr<Ping> Ping::create()
{
    return std::shared_ptr<Ping>{ new Ping{} };
}

boost::future<Timestamp_t> Ping::future()
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

}    // namespace Proto2::OutcomingRequestContext
