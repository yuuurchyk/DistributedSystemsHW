#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "frame/frame.h"
#include "incomingrequestcontext/abstractincomingrequestcontext.h"
#include "proto2/signal.h"
#include "response/abstractresponse.h"
#include "socketwrapper/socketwrapper.h"

namespace Proto2
{
class IncomingRequestsManager : public std::enable_shared_from_this<IncomingRequestsManager>,
                                private logger::Entity<IncomingRequestsManager>
{
    DISABLE_COPY_MOVE(IncomingRequestsManager)
public:
    [[nodiscard]] static std::shared_ptr<IncomingRequestsManager> create(std::shared_ptr<SocketWrapper> socketWrapper);

    using Context_t = IncomingRequestContext::AbstractIncomingRequestContext;

    void registerContext(size_t responseId, std::shared_ptr<Context_t> context);

public:    // signals
    signal<Frame::RequestFrame> incomingRequestFrame;

private:
    struct PendingResponse
    {
        DISABLE_COPY_MOVE(PendingResponse)

        [[nodiscard]] static std::unique_ptr<PendingResponse>
            create(size_t responseId, std::shared_ptr<Response::AbstractResponse> response);

        size_t                                      responseId;
        std::shared_ptr<Response::AbstractResponse> response;

    private:
        PendingResponse(size_t, std::shared_ptr<Response::AbstractResponse>);
    };

    IncomingRequestsManager(std::shared_ptr<SocketWrapper>);

    void onIncomingFrame(boost::asio::const_buffer frame);

private:
    boost::asio::io_context             &ioContext_;
    const std::shared_ptr<SocketWrapper> socketWrapper_;
};

}    // namespace Proto2
