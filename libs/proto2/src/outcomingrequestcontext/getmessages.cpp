#include "proto2/outcomingrequestcontext/getmessages.h"

#include "proto2/response/getmessages.h"

namespace Proto2::OutcomingRequestContext
{

std::unique_ptr<GetMessages> GetMessages::create()
{
    return std::unique_ptr<GetMessages>{ new GetMessages{} };
}

boost::future<std::vector<std::string>> GetMessages::future()
{
    return promise_.get_future();
}

void GetMessages::onResponseRecieved(boost::asio::const_buffer payload)
{
    if (promiseFilled())
        return;

    auto response = Response::GetMessages::fromPayload(payload);
    if (response == nullptr)
    {
        invalidate(InvalidationReason::BAD_FRAME);
    }
    else
    {
        promiseMarkFilled();
        promise_.set_value(response->flushMessages());
    }
}

void GetMessages::invalidate(InvalidationReason reason)
{
    invalidatePromise(promise_, reason);
}

}    // namespace Proto2::OutcomingRequestContext
