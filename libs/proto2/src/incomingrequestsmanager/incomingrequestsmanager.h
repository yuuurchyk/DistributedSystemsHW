#pragma once

#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "frame/frame.h"
#include "incomingrequestcontext/abstractincomingrequestcontext.h"
#include "request/abstractrequest.h"
#include "socketwrapper/socketwrapper.h"

namespace Proto2
{
class IncomingRequestsManager : public std::enable_shared_from_this<IncomingRequestsManager>,
                                private logger::Entity<IncomingRequestsManager>
{
    DISABLE_COPY_MOVE(IncomingRequestsManager)
public:
    [[nodiscard]] static std::shared_ptr<IncomingRequestsManager> create(std::shared_ptr<SocketWrapper>);

    boost::signals2::signal<void(Frame::RequestFrame)> incomingRequest;

    void addIncomingRequest(
        size_t id,
        std::shared_ptr<Request::AbstractRequest>,
        std::shared_ptr<IncomingRequestContext::AbstractIncomingRequestContext>);

private:
    IncomingRequestsManager(std::shared_ptr<SocketWrapper>);

    void onIncomingFrame(boost::asio::const_buffer frame);

    struct Pending
    {
        DISABLE_COPY_DEFAULT_MOVE(Pending)

        Pending(
            size_t id,
            std::shared_ptr<Request::AbstractRequest>,
            std::shared_ptr<IncomingRequestContext::AbstractIncomingRequestContext>);

        size_t                                                                  id;
        std::shared_ptr<Request::AbstractRequest>                               request;
        std::shared_ptr<IncomingRequestContext::AbstractIncomingRequestContext> context;
    };

    boost::asio::io_context             &ioContext_;
    const std::shared_ptr<SocketWrapper> socketWrapper_;

    std::unordered_map<size_t, Pending> requests_;
};

}    // namespace Proto2
