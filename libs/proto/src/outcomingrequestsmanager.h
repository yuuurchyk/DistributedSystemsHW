#pragma once

#include <atomic>
#include <memory>
#include <unordered_map>

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "reflection/deserialization.h"

#include "proto/concepts.h"
#include "proto/proto.h"
#include "socketwrapper.h"

namespace Proto
{
/**
 * @brief manages outcoming requests of SocketWrapper
 *
 */
class OutcomingRequestsManager : public std::enable_shared_from_this<OutcomingRequestsManager>,
                                 public logger::Entity<OutcomingRequestsManager>
{
public:
    [[nodiscard]] static std::shared_ptr<OutcomingRequestsManager>
        create(std::shared_ptr<SocketWrapper>, size_t sendTimeoutMs);
    // note: should be done after construction to avoid enable_shared_from_this issues
    void registerConnections();

    ~OutcomingRequestsManager();

    template <Concepts::Request Request>
    boost::future<typename Concepts::Response_t<Request>> send(std::shared_ptr<const Request>);

private:
    struct AbstractPendingRequest
    {
        DISABLE_COPY_MOVE(AbstractPendingRequest)

        AbstractPendingRequest()          = default;
        virtual ~AbstractPendingRequest() = default;

        virtual bool readResponseBody(Reflection::DeserializationContext<boost::asio::const_buffer> &) = 0;

        virtual void invalidateBadFrame()     = 0;
        virtual void invalidateDisconnected() = 0;
        virtual void invalidateTimeout()      = 0;
    };
    template <Concepts::Request Request>
    struct PendingRequest : final AbstractPendingRequest
    {
        bool readResponseBody(Reflection::DeserializationContext<boost::asio::const_buffer> &context) override;

        void invalidateBadFrame() override;
        void invalidateDisconnected() override;
        void invalidateTimeout() override;

        boost::promise<typename Concepts::Response_t<Request>> promise;
    };

    OutcomingRequestsManager(std::shared_ptr<SocketWrapper>, size_t sendTimeoutMs);

    size_t getNextRequestId();

    void onInvalidated();
    void onIncomingBuffer(boost::asio::const_buffer);

    const size_t                   sendTimeoutMs_;
    std::shared_ptr<SocketWrapper> socketWrapper_;
    std::atomic<size_t>            requestIdCounter_;

    std::unordered_map<size_t /* requestId */, std::shared_ptr<AbstractPendingRequest>>      pendingRequests_;
    std::unordered_map<size_t /* requestId */, std::shared_ptr<boost::asio::deadline_timer>> pendingRequestsTimers_;

    boost::signals2::connection incomingBufferConnection_;
    boost::signals2::connection invalidatedConnection_;
};

}    // namespace Proto

#include "outcomingrequestsmanager_impl.h"
