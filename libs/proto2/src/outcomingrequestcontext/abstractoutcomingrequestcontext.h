#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "utils/copymove.h"

#include "proto2/exceptions.h"

namespace Proto2::OutcomingRequestContext
{
class AbstractOutcomingRequestContext
{
    DISABLE_COPY_MOVE(AbstractOutcomingRequestContext)
public:
    AbstractOutcomingRequestContext()          = default;
    virtual ~AbstractOutcomingRequestContext() = default;

    // TODO: handle promiseMarkFilled() and promiseFilled() solely in this class
    // write virtual onResponseRecievedImpl(), invalidateImpl(), leave these not virtual
    virtual void onResponseRecieved(boost::asio::const_buffer payload) = 0;

    enum class InvalidationReason
    {
        TIMEOUT,
        BAD_FRAME,
        PEER_DISCONNECTED
    };

    virtual void invalidate(InvalidationReason) = 0;

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
            throw TimeoutException{};
        case InvalidationReason::BAD_FRAME:
            throw BadFrameException{};
        case InvalidationReason::PEER_DISCONNECTED:
            throw PeerDisconnectedException{};
        }
    }
    catch (const ResponseException &)
    {
        promiseMarkFilled();
        promise.set_exception(std::current_exception());
    }
}

}    // namespace Proto2::OutcomingRequestContext
