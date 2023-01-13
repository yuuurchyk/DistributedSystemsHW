#pragma once

#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

#include "logger/logger.h"
#include "utils/copymove.h"
#include "utils/duration.h"

#include "outcomingrequestcontext/abstractoutcomingrequestcontext.h"
#include "request/abstractrequest.h"
#include "socketwrapper/socketwrapper.h"

namespace Proto
{
class OutcomingRequestsManager final : public std::enable_shared_from_this<OutcomingRequestsManager>,
                                       private logger::StringIdEntity<OutcomingRequestsManager>
{
    DISABLE_COPY_MOVE(OutcomingRequestsManager)
public:
    [[nodiscard]] static std::shared_ptr<OutcomingRequestsManager>
        create(std::string id, std::shared_ptr<SocketWrapper>, Utils::duration_milliseconds_t responseTimeout);

    ~OutcomingRequestsManager();

    using Request_t = std::shared_ptr<Request::AbstractRequest>;
    using Context_t = std::shared_ptr<OutcomingRequestContext::AbstractOutcomingRequestContext>;

    // thread safe
    void sendRequest(Request_t request, Context_t context, Utils::duration_milliseconds_t artificialDelay);

private:
    OutcomingRequestsManager(
        std::string id,
        std::shared_ptr<SocketWrapper>,
        Utils::duration_milliseconds_t responseTimeout);

    void establishConnections();

    void sendRequestImpl(Request_t, Context_t, Utils::duration_milliseconds_t artificialDelay);

    void onIncomingFrame(boost::asio::const_buffer frame);
    void onResponseRecieved(size_t requestId, boost::asio::const_buffer payload);

    void invalidatePendingRequest(size_t requestId, OutcomingRequestContext::InvalidationReason);
    void invalidateAllPendingRequests(OutcomingRequestContext::InvalidationReason);

private:
    boost::asio::io_context             &ioContext_;
    const std::shared_ptr<SocketWrapper> socketWrapper_;

    const Utils::duration_milliseconds_t responseTimeout_{};

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

}    // namespace Proto
