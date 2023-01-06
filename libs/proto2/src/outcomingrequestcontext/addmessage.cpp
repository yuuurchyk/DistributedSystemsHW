#include "proto2/outcomingrequestcontext/addmessage.h"

#include <cassert>

#include "proto2/exceptions.h"

namespace Proto2::OutcomingRequestContext
{

std::shared_ptr<AddMessage> AddMessage::create()
{
    return std::shared_ptr<AddMessage>{ new AddMessage{} };
}

boost::future<Response::AddMessage::Status> AddMessage::future()
{
    return promise_.get_future();
}

void AddMessage::onResponseRecieved(boost::asio::const_buffer payload)
{
    if (promiseFilled())
        return;

    auto response = Response::AddMessage::fromPayload(payload);
    if (response == nullptr)
    {
        invalidate(InvalidationReason::BAD_FRAME);
    }
    else
    {
        promiseMarkFilled();
        promise_.set_value(response->status());
    }
}

void AddMessage::invalidate(InvalidationReason reason)
{
    invalidatePromise(promise_, reason);
}

}    // namespace Proto2::OutcomingRequestContext
