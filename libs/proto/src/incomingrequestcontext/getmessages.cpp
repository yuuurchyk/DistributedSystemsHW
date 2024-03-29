#include "incomingrequestcontext/getmessages.h"

#include <utility>

#include "net-utils/thenpost.h"

#include "response/getmessages.h"

namespace Proto::IncomingRequestContext
{
std::shared_ptr<GetMessages> GetMessages::create(boost::asio::io_context &executionContext)
{
    auto self = std::shared_ptr<GetMessages>{ new GetMessages{ executionContext } };

    self->connectPromise();

    return self;
}

boost::promise<std::vector<std::string_view>> GetMessages::flushPromise()
{
    return std::move(promise_);
}

void GetMessages::connectPromise()
{
    NetUtils::thenPost(
        promise_.get_future(),
        executionContext_,
        [this, weakSelf = weak_from_this()](boost::future<std::vector<std::string_view>> future)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (future.has_value())
                responseRecieved(Response::GetMessages::create(future.get()));
            else
                invalidResponseRecieved();
        });
}

}    // namespace Proto::IncomingRequestContext
