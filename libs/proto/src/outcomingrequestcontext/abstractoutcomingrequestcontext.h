#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "utils/copymove.h"

#include "proto/exceptions.h"

#include "outcomingrequestcontext/invalidationreason.h"

namespace Proto::OutcomingRequestContext
{
class AbstractOutcomingRequestContext
{
    DISABLE_COPY_MOVE(AbstractOutcomingRequestContext)
public:
    AbstractOutcomingRequestContext()          = default;
    virtual ~AbstractOutcomingRequestContext() = default;

    virtual void onResponseRecieved(boost::asio::const_buffer payload) = 0;
    virtual void invalidate(InvalidationReason)                        = 0;

protected:
    void promiseMarkFilled();
    bool promiseFilled() const;

    template <typename T>
    void invalidatePromise(boost::promise<T> &, InvalidationReason);

private:
    bool promiseFilled_{};
};

template <typename T>
void AbstractOutcomingRequestContext::invalidatePromise(boost::promise<T> &promise, InvalidationReason reason)
{
    if (promiseFilled())
        return;

    try
    {
        switch (reason)
        {
        case InvalidationReason::TIMEOUT:
            BOOST_THROW_EXCEPTION(Exceptions::Timeout{});
        case InvalidationReason::BAD_RESPONSE_FRAME:
            BOOST_THROW_EXCEPTION(Exceptions::BadResponseFrame{});
        case InvalidationReason::PEER_DISCONNECTED:
            BOOST_THROW_EXCEPTION(Exceptions::PeerDisconnected{});
        }
    }
    catch (const Exceptions::ResponseException &)
    {
        promiseMarkFilled();
        promise.set_exception(boost::current_exception());
    }
}

}    // namespace Proto::OutcomingRequestContext
