#include "incomingrequestsmanager/incomingrequestsmanager.h"

#include <utility>

#include "frame/frame.h"

using error_code = boost::system::error_code;

namespace Proto2
{
std::shared_ptr<IncomingRequestsManager>
    IncomingRequestsManager::create(std::string id, std::shared_ptr<SocketWrapper> socketWrapper)
{
    auto self = std::shared_ptr<IncomingRequestsManager>{ new IncomingRequestsManager{ std::move(id),
                                                                                       std::move(socketWrapper) } };

    self->establishConnections();

    return self;
}

void IncomingRequestsManager::registerContext(size_t responseId, Context_t context)
{
    if (const auto it = registeredContexts_.find(responseId); it != registeredContexts_.end())
    {
        EN_LOGE << "registerContext called with id that is already registered, skipping";
        return;
    }

    context->responseRecieved.connect(Utils::slot_type<std::shared_ptr<Response::AbstractResponse>>{
        &IncomingRequestsManager::onResponseRecieved, this, responseId, boost::placeholders::_1 }
                                          .track_foreign(weak_from_this()));

    context->invalidResponseRecieved.connect(
        Utils::slot_type<>{ &IncomingRequestsManager::onInvalidResponseRecieved, this, responseId }.track_foreign(
            weak_from_this()));

    registeredContexts_.insert({ responseId, std::move(context) });
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

IncomingRequestsManager::IncomingRequestsManager(std::string id, std::shared_ptr<SocketWrapper> socketWrapper)
    : logger::StringIdEntity<IncomingRequestsManager>{ std::move(id) }, socketWrapper_{ std::move(socketWrapper) }
{
}

void IncomingRequestsManager::establishConnections()
{
    socketWrapper_->incomingFrame.connect(Utils::slot_type<boost::asio::const_buffer>{
        &IncomingRequestsManager::onIncomingFrame, this, boost::placeholders::_1 }
                                              .track_foreign(weak_from_this()));
}

void IncomingRequestsManager::onIncomingFrame(boost::asio::const_buffer frame)
{
    const auto optEventType = Frame::parseEventType(frame);
    if (!optEventType.has_value() || optEventType.value() != EventType::REQUEST)
        return;

    EN_LOGD << "onIncomingFrame: got request frame, parsing";

    auto optRequestFrame = Frame::parseRequestFrame(frame);
    if (optRequestFrame.has_value())
    {
        auto requestFrame = std::move(optRequestFrame.value());

        EN_LOGD << "incoming request frame: requestId=" << requestFrame.requestId << ", opCode=" << requestFrame.opCode;

        incomingRequestFrame(std::move(requestFrame));
    }
    else
    {
        EN_LOGW << "failed to parse request frame";
    }
}

void IncomingRequestsManager::onResponseRecieved(
    size_t                                      responseId,
    std::shared_ptr<Response::AbstractResponse> response)
{
    if (const auto it = registeredContexts_.find(responseId); it != registeredContexts_.end())
        registeredContexts_.erase(it);
    if (response == nullptr)
        return;

    EN_LOGD << "writing response for request " << responseId << " through socket";

    auto pendingResponse = PendingResponse::create(responseId, std::move(response));

    auto responseFrame = Frame::constructResponseHeaderWoOwnership(pendingResponse->responseId);
    pendingResponse->response->serializePayloadWoOwnership(responseFrame);

    socketWrapper_->writeFrame(
        std::move(responseFrame), [pendingResponse = std::move(pendingResponse)](const error_code &, size_t) {});
}

void IncomingRequestsManager::onInvalidResponseRecieved(size_t responseId)
{
    // don't send anything through socket, since recieved response is invalid
    if (const auto it = registeredContexts_.find(responseId); it != registeredContexts_.end())
        registeredContexts_.erase(it);
}

}    // namespace Proto2
