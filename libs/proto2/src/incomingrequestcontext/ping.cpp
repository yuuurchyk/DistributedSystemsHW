#include "incomingrequestcontext/ping.h"

#include <utility>

#include "net-utils/thenpost.h"

#include "response/pong.h"

namespace Proto2::IncomingRequestContext
{
std::shared_ptr<Ping> Ping::create(boost::asio::io_context &ioContext)
{
    auto self = std::shared_ptr<Ping>{ new Ping{ ioContext } };

    self->connectPromise();

    return self;
}

boost::promise<Timestamp_t> Ping::flushPromise()
{
    return std::move(promise_);
}

void Ping::connectPromise()
{
    NetUtils::thenPost(
        promise_.get_future(),
        ioContext_,
        [this, weakSelf = weak_from_this()](boost::future<Timestamp_t> future)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (future.has_value())
                responseRecieved(Response::Pong::create(future.get()));
            else
                invalidResponseRecieved();
        });
}

}    // namespace Proto2::IncomingRequestContext
