#include "proto2/endpoint.h"

#include <algorithm>
#include <utility>

#include "codes/codes.h"
#include "frame/frame.h"
#include "incomingrequestsmanager/incomingrequestsmanager.h"
#include "outcomingrequestsmanager/outcomingrequestsmanager.h"

#include "request/addmessage.h"
#include "request/getmessages.h"
#include "request/ping.h"
#include "request/secondarynodeready.h"

#include "outcomingrequestcontext/addmessage.h"
#include "outcomingrequestcontext/getmessages.h"
#include "outcomingrequestcontext/ping.h"
#include "outcomingrequestcontext/secondarynodeready.h"

#include "incomingrequestcontext/addmessage.h"
#include "incomingrequestcontext/getmessages.h"
#include "incomingrequestcontext/ping.h"
#include "incomingrequestcontext/secondarynodeready.h"

namespace
{
template <typename T>
Proto2::SharedPromise<T> makeSharedPromise(boost::promise<T> promise)
{
    return std::make_shared<boost::promise<T>>(std::move(promise));
}

}    // namespace

namespace Proto2
{
struct Endpoint::impl_t
{
    DISABLE_COPY_MOVE(impl_t)

    impl_t(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket, size_t responseTimeoutMs)
        : ioContext{ ioContext },
          socketWrapper{ SocketWrapper::create(ioContext, std::move(socket)) },
          incomingRequestsManager{ IncomingRequestsManager::create(socketWrapper) },
          outcomingRequestsManager{ OutcomingRequestsManager::create(socketWrapper, responseTimeoutMs) }
    {
    }

    boost::asio::io_context                  &ioContext;
    std::shared_ptr<SocketWrapper>            socketWrapper;
    std::shared_ptr<IncomingRequestsManager>  incomingRequestsManager;
    std::shared_ptr<OutcomingRequestsManager> outcomingRequestsManager;
};

std::shared_ptr<Endpoint> Endpoint::create(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    std::chrono::milliseconds    outcomingRequestTimeout)
{
    const auto signedCount = outcomingRequestTimeout.count();
    const auto count       = signedCount < 0 ? size_t{} : static_cast<size_t>(signedCount);

    auto self = std::shared_ptr<Endpoint>{ new Endpoint{ ioContext, std::move(socket), count } };

    self->establishConnections();

    return self;
}

Endpoint::~Endpoint() = default;

void Endpoint::run()
{
    impl().socketWrapper->run();
}

boost::future<AddMessageStatus> Endpoint::addMessage(size_t msgId, std::string_view msg)
{
    auto request = Request::AddMessage::create(msgId, msg);
    auto context = OutcomingRequestContext::AddMessage::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context));

    return future;
}

boost::future<std::vector<std::string>> Endpoint::getMessages(size_t startMsgId)
{
    auto request = Request::GetMessages::create(startMsgId);
    auto context = OutcomingRequestContext::GetMessages::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context));

    return future;
}

boost::future<Timestamp_t> Endpoint::ping(Timestamp_t pingTimestamp)
{
    auto request = Request::Ping::create(pingTimestamp);
    auto context = OutcomingRequestContext::Ping::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context));

    return future;
}

boost::future<void> Endpoint::secondaryNodeReady(std::string secondaryName)
{
    auto request = Request::SecondaryNodeReady::create(std::move(secondaryName));
    auto context = OutcomingRequestContext::SecondaryNodeReady::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context));

    return future;
}

Endpoint::Endpoint(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket, size_t responseTimeoutMs)
    : TW5VNznK_{ std::make_unique<impl_t>(ioContext, std::move(socket), responseTimeoutMs) }
{
}

void Endpoint::establishConnections()
{
    impl().socketWrapper->invalidated.connect(
        [this, weakSelf = weak_from_this()]()
        {
            const auto self = weakSelf.lock();
            if (self != nullptr)
                invalidated();
        });

    impl().incomingRequestsManager->incomingRequestFrame.connect(
        [this, weakSelf = weak_from_this()](Frame::RequestFrame requestFrame)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            switch (requestFrame.opCode)
            {
            case OpCode::ADD_MESSAGE:
            {
                auto request = Request::AddMessage::fromPayload(requestFrame.payload);
                if (request == nullptr)
                    break;

                auto context = IncomingRequestContext::AddMessage::create(impl().ioContext);
                auto promise = makeSharedPromise(context->flushPromise());
                impl().incomingRequestsManager->registerContext(requestFrame.requestId, std::move(context));

                incoming_addMessage(request->messageId(), request->flushMessage(), promise);

                break;
            }
            case OpCode::GET_MESSAGES:
            {
                auto request = Request::GetMessages::fromPayload(requestFrame.payload);
                if (request == nullptr)
                    break;

                auto context = IncomingRequestContext::GetMessages::create(impl().ioContext);
                auto promise = makeSharedPromise(context->flushPromise());
                impl().incomingRequestsManager->registerContext(requestFrame.requestId, std::move(context));

                incoming_getMessages(request->startMessageId(), promise);

                break;
            }
            case OpCode::PING_PONG:
            {
                auto request = Request::Ping::fromPayload(requestFrame.payload);
                if (request == nullptr)
                    break;

                auto context = IncomingRequestContext::Ping::create(impl().ioContext);
                auto promise = makeSharedPromise(context->flushPromise());
                impl().incomingRequestsManager->registerContext(requestFrame.requestId, std::move(context));

                incoming_ping(request->timestamp(), promise);

                break;
            }
            case OpCode::SECONDARY_NODE_READY:
            {
                auto request = Request::SecondaryNodeReady::fromPayload(requestFrame.payload);
                if (request == nullptr)
                    break;

                auto context = IncomingRequestContext::SecondaryNodeReady::create(impl().ioContext);
                auto promise = makeSharedPromise(context->flushPromise());
                impl().incomingRequestsManager->registerContext(requestFrame.requestId, std::move(context));

                incoming_secondaryNodeReady(request->flushSecondaryNodeName(), promise);

                break;
            }
            }
        });
}

auto Endpoint::impl() -> impl_t &
{
    return *TW5VNznK_;
}

auto Endpoint::impl() const -> const impl_t &
{
    return *TW5VNznK_;
}

}    // namespace Proto2
