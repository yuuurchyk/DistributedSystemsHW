#include "outcomingrequestcontext/addmessage.h"

#include <cassert>

#include "proto2/exceptions.h"
#include "response/addmessage.h"

namespace Proto2::OutcomingRequestContext
{
std::shared_ptr<AddMessage> AddMessage::create()
{
    return std::shared_ptr<AddMessage>{ new AddMessage{} };
}

boost::future<AddMessageStatus> AddMessage::get_future()
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
