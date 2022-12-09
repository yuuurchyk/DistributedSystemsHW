#include "incomingrequestsmanager.h"

#include <utility>

#include "reflection/serialization.h"

#include "frame.h"

namespace Proto
{
template <Concepts::Response Response>
void IncomingRequestsManager::addPendingResponse(size_t requestId, boost::promise<Response> &promise)
{
    if (const auto it = pendingResponses_.find(requestId); it != pendingResponses_.end())
        return;

    auto pendingResponse = std::make_shared<PendingResponse>();
    pendingResponses_.insert({ requestId, std::move(pendingResponse) });

    promise.get_future().then(
        [this,
         requestId,
         pendingResponse,
         weakSelf = weak_from_this(),
         weakPendingResponse =
             std::weak_ptr<PendingResponse>{ pendingResponse }](boost::future<Response> responseFuture)
        {
            if (!responseFuture.has_value())
                return;

            auto self            = weakSelf.lock();
            auto pendingResponse = weakPendingResponse.lock();

            if (self == nullptr || pendingResponse == nullptr)
                return;

            auto context = std::make_shared<Reflection::SerializationContext>();

            auto responseFrame = std::make_shared<ResponseHeader>(requestId);
            auto response      = std::make_shared<Response>(responseFuture.get());

            context->serializeAndHold(std::move(responseFrame));
            context->serializeAndHold(std::move(response));

            boost::asio::post(
                socketWrapper_->executor(),
                [this, requestId, pendingResponse, context = std::move(context), self]()
                {
                    if (const auto it = pendingResponses_.find(requestId); it != pendingResponses_.end())
                        pendingResponses_.erase(it);

                    if (pendingResponse->invalidated_)
                        return;

                    socketWrapper_->send(std::move(context));
                });
        });
}

std::shared_ptr<IncomingRequestsManager> IncomingRequestsManager::create(std::shared_ptr<SocketWrapper> socketWrapper)
{
    auto res = std::shared_ptr<IncomingRequestsManager>{ new IncomingRequestsManager{ std::move(socketWrapper) } };
    res->registerConnections();
    return res;
}

void IncomingRequestsManager::registerConnections()
{
    incomingBufferConnection_ = socketWrapper_->incomingBuffer.connect(
        [this, weakSelf = weak_from_this()](boost::asio::const_buffer buffer)
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
                return;

            onIncomingBuffer(buffer);
        });
    invalidatedConnection_ = socketWrapper_->invalidated.connect(
        [this, weakSelf = weak_from_this()]()
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
                return;

            onInvalidated();
        });
}

IncomingRequestsManager::~IncomingRequestsManager()
{
    incomingBufferConnection_.disconnect();
    invalidatedConnection_.disconnect();

    for (auto &[_, pendingResponse] : pendingResponses_)
        pendingResponse->invalidated_ = true;
    pendingResponses_.clear();
}

IncomingRequestsManager::IncomingRequestsManager(std::shared_ptr<SocketWrapper> socketWrapper)
    : socketWrapper_{ std::move(socketWrapper) }
{
}

template <Concepts::Request Request>
void IncomingRequestsManager::readIncomingRequest(
    size_t                                                         requestId,
    Reflection::DeserializationContext<boost::asio::const_buffer> &body,
    RequestSignal<Request>                                        &incomingRequestSignal)
{
    using Response = typename Concepts::Response_t<Request>;

    auto optRequest = body.deserialize<Request>();

    if (!optRequest.has_value() || !body.atEnd())
    {
        EN_LOGW << "Failed to read incoming request";
        return;
    }

    auto request = std::make_shared<Request>(std::move(optRequest.value()));
    auto promise = std::make_shared<boost::promise<Response>>();

    addPendingResponse(requestId, *promise);

    incomingRequestSignal(std::move(request), std::move(promise));
}

void IncomingRequestsManager::onInvalidated()
{
    for (auto &[_, pendingResponse] : pendingResponses_)
        pendingResponse->invalidated_ = true;
    pendingResponses_.clear();
}

void IncomingRequestsManager::onIncomingBuffer(boost::asio::const_buffer buffer)
{
    auto context = Reflection::DeserializationContext{ buffer };

    const auto optIncomingHeader = context.deserialize<IncomingHeader>();

    if (!optIncomingHeader.has_value())
        return;

    const auto &incomingHeader = optIncomingHeader.value();

    if (incomingHeader.eventType != EventType::REQUEST)
        return;

    const auto requestId = incomingHeader.requestId;

    const auto optOpCode = context.deserialize<OpCode>();

    if (!optOpCode.has_value())
    {
        EN_LOGW << "Failed to read incoming request opcode";
        return;
    }

    const auto opCode = optOpCode.value();

    switch (opCode)
    {
    case OpCode::ADD_MESSAGE:
        return readIncomingRequest<Request::AddMessage>(requestId, context, incoming_addMessage);
    case OpCode::GET_MESSAGES:
        return readIncomingRequest<Request::GetMessages>(requestId, context, incoming_getMessages);
    default:
        EN_LOGW << "wrong opcode for incoming request";
        break;
    }
}

}    // namespace Proto
