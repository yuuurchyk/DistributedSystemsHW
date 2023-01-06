#include "proto2/incomingrequestcontext/addmessage.h"

#include <utility>

#include "net-utils/thenpost.h"

#include "proto2/response/addmessage.h"

namespace Proto2::IncomingRequestContext
{

std::shared_ptr<AddMessage> AddMessage::create(boost::asio::io_context &ioContext)
{
    auto self = std::shared_ptr<AddMessage>{ new AddMessage{ ioContext } };

    self->connectPromise();

    return self;
}

boost::promise<AddMessageStatus> AddMessage::flushPromise()
{
    return std::move(promise_);
}

void AddMessage::connectPromise()
{
    NetUtils::thenPost(
        promise_.get_future(),
        ioContext_,
        [this, weakSelf = weak_from_this()](boost::future<AddMessageStatus> future)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (future.has_value())
                responseRecieved(Response::AddMessage::create(future.get()));
            else
                invalidResponseRecieved();
        });
}

}    // namespace Proto2::IncomingRequestContext
