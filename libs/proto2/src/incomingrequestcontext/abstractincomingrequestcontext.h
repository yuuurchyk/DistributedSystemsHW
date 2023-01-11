#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "utils/copymove.h"
#include "utils/signal.h"

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

    // note: successor classes should emit these signals in thread that runs
    // execution context
    Utils::signal<std::shared_ptr<Response::AbstractResponse>> responseRecieved;
    Utils::signal<>                                            invalidResponseRecieved;

protected:
    /**
     * @brief Construct a new Abstract Incoming Request Context object
     *
     * @param ioContext - signals will be emitted in a thread where
     * provided @p ioContext runs
     */
    AbstractIncomingRequestContext(boost::asio::io_context &executionContext);

    virtual void connectPromise() = 0;

    boost::asio::io_context &executionContext_;
};

}    // namespace Proto2::IncomingRequestContext
