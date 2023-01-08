#include "incomingrequestsmanager/incomingrequestsmanager.h"

#include <utility>

#include "frame/frame.h"

using error_code = boost::system::error_code;

namespace Proto2
{
std::shared_ptr<IncomingRequestsManager> IncomingRequestsManager::create(std::shared_ptr<SocketWrapper> socketWrapper)
{
    return std::shared_ptr<IncomingRequestsManager>{ new IncomingRequestsManager{ std::move(socketWrapper) } };
}

void IncomingRequestsManager::registerContext(size_t responseId, std::shared_ptr<Context_t> context)
{
    context->responseRecieved.connect(
        [this, responseId, weakSelf = weak_from_this()](std::shared_ptr<Response::AbstractResponse> response)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;
            if (response == nullptr)
                return;

            boost::asio::post(
                ioContext_,
                [this, responseId, response = std::move(response), weakSelf = weak_from_this()]() mutable
                {
                    const auto self = weakSelf.lock();
                    if (self == nullptr)
                        return;

                    auto pendingResponse = PendingResponse::create(responseId, std::move(response));

                    auto responseFrame = Frame::constructResponseHeaderWoOwnership(pendingResponse->responseId);
                    pendingResponse->response->serializePayloadWoOwnership(responseFrame);

                    socketWrapper_->writeFrame(
                        std::move(responseFrame),
                        [pendingResponse = std::move(pendingResponse)](const error_code &, size_t) {});
                });
        });
}

auto IncomingRequestsManager::PendingResponse::create(
    size_t                                      responseId,
    std::shared_ptr<Response::AbstractResponse> response) -> std::shared_ptr<PendingResponse>
{
    return std::shared_ptr<PendingResponse>{ new PendingResponse{ responseId, std::move(response) } };
}

IncomingRequestsManager::PendingResponse::PendingResponse(
    size_t                                      responseId,
    std::shared_ptr<Response::AbstractResponse> response)
    : responseId{ responseId }, response{ std::move(response) }
{
}

IncomingRequestsManager::IncomingRequestsManager(std::shared_ptr<SocketWrapper> socketWrapper)
    : ioContext_{ socketWrapper->ioContext() }, socketWrapper_{ std::move(socketWrapper) }
{
    socketWrapper_->incomingFrame.connect(
        [this, weakSelf = weak_from_this()](boost::asio::const_buffer frame)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            onIncomingFrame(frame);
        });
}

void IncomingRequestsManager::onIncomingFrame(boost::asio::const_buffer frame)
{
    auto optRequestFrame = Frame::parseRequestFrame(frame);
    if (optRequestFrame.has_value())
        incomingRequestFrame(std::move(optRequestFrame.value()));
}

}    // namespace Proto2
