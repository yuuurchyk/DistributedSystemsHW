#include "proto2/outcomingrequestcontext/secondarynodeready.h"

#include "response/secondarynodeready.h"

namespace Proto2::OutcomingRequestContext
{
std::shared_ptr<SecondaryNodeReady> SecondaryNodeReady::create()
{
    return std::shared_ptr<SecondaryNodeReady>{ new SecondaryNodeReady{} };
}

boost::future<void> SecondaryNodeReady::future()
{
    return promise_.get_future();
}

void SecondaryNodeReady::onResponseRecieved(boost::asio::const_buffer payload)
{
    if (promiseFilled())
        return;

    auto response = Response::SecondaryNodeReady::fromPayload(payload);
    if (response == nullptr)
    {
        invalidate(InvalidationReason::BAD_FRAME);
    }
    else
    {
        promiseMarkFilled();
        promise_.set_value();
    }
}

void SecondaryNodeReady::invalidate(InvalidationReason reason)
{
    invalidatePromise(promise_, reason);
}

}    // namespace Proto2::OutcomingRequestContext
