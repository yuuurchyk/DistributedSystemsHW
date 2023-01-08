#pragma once

#include <memory>
#include <queue>
#include <unordered_map>

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
    [[nodiscard]] static std::shared_ptr<OutcomingRequestsManager>
        create(std::shared_ptr<SocketWrapper>, size_t responseTimeoutMs);

    ~OutcomingRequestsManager();

    using Request_t = std::shared_ptr<Request::AbstractRequest>;
    using Context_t = std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext>;

    // thread safe
    void sendRequest(Request_t request, Context_t context);

private:
    OutcomingRequestsManager(std::shared_ptr<SocketWrapper>, size_t responseTimeoutMs);

    void establishConnections();

    void sendRequestImpl(Request_t, Context_t);

    void onExpired(size_t requestId);
    void onResponseRecieved(size_t requestId, boost::asio::const_buffer payload);
    void onIncomingFrame(boost::asio::const_buffer frame);

private:
    boost::asio::io_context             &ioContext_;
    const std::shared_ptr<SocketWrapper> socketWrapper_;

    const size_t responseTimeoutMs_{};

    size_t requestIdCounter_{};

    struct PendingRequest
    {
        DISABLE_COPY_MOVE(PendingRequest)

        [[nodiscard]] static std::shared_ptr<PendingRequest>
            create(boost::asio::io_context &ioContext, size_t requestId, Request_t request, Context_t context);

        size_t                      requestId;
        boost::asio::deadline_timer timeoutTimer;

        Request_t request;
        Context_t context;

    private:
        PendingRequest(boost::asio::io_context &ioContext, size_t requestId, Request_t request, Context_t context);
    };

    std::unordered_map<size_t, std::shared_ptr<PendingRequest>> pendingRequests_;
};

}    // namespace Proto2
