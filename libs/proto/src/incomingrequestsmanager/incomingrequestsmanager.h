#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"
#include "utils/signal.h"

#include "frame/frame.h"
#include "incomingrequestcontext/abstractincomingrequestcontext.h"
#include "response/abstractresponse.h"
#include "socketwrapper/socketwrapper.h"

namespace Proto
{
/**
 * How to use the class?
 * 1. react to incomingRequestFrame() signal
 * 2. construct the response context and register it via registerContext()
 *
 * The class internally reacts to context signals and writes response through socket wrapper
 *
 * @note there are no thread safe methods, all calls should be done within
 * socket wrapper execution context
 */
class IncomingRequestsManager : public std::enable_shared_from_this<IncomingRequestsManager>,
                                private logger::StringIdEntity<IncomingRequestsManager>
{
    DISABLE_COPY_MOVE(IncomingRequestsManager)
public:
    [[nodiscard]] static std::shared_ptr<IncomingRequestsManager>
        create(std::string id, std::shared_ptr<SocketWrapper> socketWrapper);

    using Context_t = std::shared_ptr<IncomingRequestContext::AbstractIncomingRequestContext>;

    void registerContext(size_t responseId, Context_t context);

public:    // signals
    Utils::signal<Frame::RequestFrame> incomingRequestFrame;

private:
    struct PendingResponse
    {
        DISABLE_COPY_MOVE(PendingResponse)

        [[nodiscard]] static std::shared_ptr<PendingResponse>
            create(size_t responseId, std::shared_ptr<Response::AbstractResponse> response);

        size_t                                      responseId;
        std::shared_ptr<Response::AbstractResponse> response;

    private:
        PendingResponse(size_t, std::shared_ptr<Response::AbstractResponse>);
    };

    IncomingRequestsManager(std::string id, std::shared_ptr<SocketWrapper>);

    void establishConnections();

    void onIncomingFrame(boost::asio::const_buffer frame);

    void onResponseRecieved(size_t responseId, std::shared_ptr<Response::AbstractResponse> response);
    void onInvalidResponseRecieved(size_t responseId);

private:
    const std::shared_ptr<SocketWrapper> socketWrapper_;

    std::unordered_map<size_t, Context_t> registeredContexts_;
};

}    // namespace Proto
