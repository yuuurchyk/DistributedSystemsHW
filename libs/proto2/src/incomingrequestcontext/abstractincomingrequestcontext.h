#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "utils/copymove.h"

#include "proto2/signal.h"
#include "response/abstractresponse.h"

namespace Proto2::IncomingRequestContext
{
class AbstractIncomingRequestContext : public std::enable_shared_from_this<AbstractIncomingRequestContext>
{
    DISABLE_COPY_MOVE(AbstractIncomingRequestContext)
public:
    virtual ~AbstractIncomingRequestContext() = default;

    // note: all successors should have flushPromise() method,
    // which will return the promise to response

    // NOTE: this signals should be emitted within the thread that runs the
    // io context
    signal<std::shared_ptr<Response::AbstractResponse>> responseRecieved;
    signal<>                                            invalidResponseRecieved;

protected:
    /**
     * @brief Construct a new Abstract Incoming Request Context object
     *
     * @param ioContext - signals will be emitted in a thread where
     * provided @p ioContext runs
     */
    AbstractIncomingRequestContext(boost::asio::io_context &ioContext);

    virtual void connectPromise() = 0;

    boost::asio::io_context &ioContext_;
};

}    // namespace Proto2::IncomingRequestContext
