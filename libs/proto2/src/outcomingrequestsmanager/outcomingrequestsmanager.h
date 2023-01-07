#pragma once

#include <atomic>
#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "outcomingrequestcontext/abstractoutcomingrequestcontext.h"
#include "request/abstractrequest.h"
#include "socketwrapper/socketwrapper.h"

namespace Proto2
{
class OutcomingRequestsManager : public std::enable_shared_from_this<OutcomingRequestsManager>,
                                 private logger::Entity<OutcomingRequestsManager>
{
    DISABLE_COPY_MOVE(OutcomingRequestsManager)
public:
    [[nodiscard]] std::shared_ptr<OutcomingRequestsManager>
        create(std::shared_ptr<SocketWrapper> socketWrapper, size_t responseTimeoutMs);

    // thread safe
    void send(
        std::shared_ptr<Request::AbstractRequest>,
        std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext>);

private:
    OutcomingRequestsManager(boost::asio::io_context &, std::shared_ptr<SocketWrapper>, size_t responseTimeoutMs);

    size_t getNextRequestId();

    struct Pending;
    void sendImpl(Pending);

    void onIncomingFrame(boost::asio::const_buffer frame);

    void onExpired(size_t id);
    void onResponseRecieved(size_t id, boost::asio::const_buffer payload);

    void onInvalidated();

private:
    boost::asio::io_context             &ioContext_;
    const std::shared_ptr<SocketWrapper> socketWrapper_;

    std::atomic<size_t> requestIdCounter_;
    size_t requestIdBuffer_;

    const size_t responseTimeoutMs_;

    struct Pending
    {
        DISABLE_COPY_DEFAULT_MOVE(Pending)

        Pending(
            boost::asio::io_context                                                  &ioContext,
            size_t                                                                    id,
            std::shared_ptr<Request::AbstractRequest>                                 request,
            std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext> context);

        size_t                                                                    id;
        boost::asio::deadline_timer                                               timeoutTimer;
        std::shared_ptr<Request::AbstractRequest>                                 request;
        std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext> context;
    };

    std::unordered_map<size_t /* requestId */, Pending> requests_;
};

}    // namespace Proto2
