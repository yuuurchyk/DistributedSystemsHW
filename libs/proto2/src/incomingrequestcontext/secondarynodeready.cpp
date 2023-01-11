#include "incomingrequestcontext/secondarynodeready.h"

#include <utility>

#include "net-utils/thenpost.h"

#include "response/secondarynodeready.h"

namespace Proto2::IncomingRequestContext
{

std::shared_ptr<SecondaryNodeReady> SecondaryNodeReady::create(boost::asio::io_context &ioContext)
{
    auto self = std::shared_ptr<SecondaryNodeReady>{ new SecondaryNodeReady{ ioContext } };

    self->connectPromise();

    return self;
}

boost::promise<void> SecondaryNodeReady::flushPromise()
{
    return std::move(promise_);
}

void SecondaryNodeReady::connectPromise()
{
    NetUtils::thenPost(
        promise_.get_future(),
        ioContext_,
        [this, weakSelf = weak_from_this()](boost::future<void> future)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (future.has_value())
                responseRecieved(Response::SecondaryNodeReady::create());
            else
                invalidResponseRecieved();
        });
}

}    // namespace Proto2::IncomingRequestContext
