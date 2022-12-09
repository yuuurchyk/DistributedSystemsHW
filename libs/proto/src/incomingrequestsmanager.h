#pragma once

#include <memory>
#include <unordered_map>

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "reflection/deserialization.h"
#include "utils/copymove.h"

#include "proto/concepts.h"
#include "proto/proto.h"
#include "socketwrapper.h"

namespace Proto
{
class IncomingRequestsManager : public std::enable_shared_from_this<IncomingRequestsManager>,
                                private logger::Entity<IncomingRequestsManager>
{
    DISABLE_COPY_MOVE(IncomingRequestsManager)

public:
    template <typename T>
    using signal = boost::signals2::signal<T>;
    template <Concepts::Request Request>
    using RequestSignal =
        signal<void(std::shared_ptr<Request>, std::shared_ptr<boost::promise<typename Concepts::Response_t<Request>>>)>;

    [[nodiscard]] static std::shared_ptr<IncomingRequestsManager> create(std::shared_ptr<SocketWrapper>);

    ~IncomingRequestsManager();

    RequestSignal<Request::AddMessage>  incoming_addMessage;
    RequestSignal<Request::GetMessages> incoming_getMessages;

private:
    struct PendingResponse
    {
        DISABLE_COPY_MOVE(PendingResponse)

        PendingResponse() = default;

        bool invalidated_{};
    };
    template <Concepts::Response Response>
    void addPendingResponse(size_t requestId, boost::promise<Response> &);

    IncomingRequestsManager(std::shared_ptr<SocketWrapper>);

    void registerConnections();

    template <Concepts::Request Request>
    void readIncomingRequest(
        size_t                                                         requestId,
        Reflection::DeserializationContext<boost::asio::const_buffer> &body,
        RequestSignal<Request>                                        &incomingRequestSignal);

    void onInvalidated();
    void onIncomingBuffer(boost::asio::const_buffer);

    std::shared_ptr<SocketWrapper> socketWrapper_;

    std::unordered_map<size_t, std::shared_ptr<PendingResponse>> pendingResponses_;

    boost::signals2::connection incomingBufferConnection_;
    boost::signals2::connection invalidatedConnection_;
};

}    // namespace Proto
