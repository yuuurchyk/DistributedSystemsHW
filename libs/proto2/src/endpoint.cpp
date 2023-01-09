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

boost::future<AddMessageStatus> Endpoint::send_addMessage(size_t msgId, std::string_view msg)
{
    EN_LOGI << "sending addMessage(msgId=" << msgId << ", msg=" << msg << ")";

    auto request = Request::AddMessage::create(msgId, msg);
    auto context = OutcomingRequestContext::AddMessage::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context));

    return future;
}

boost::future<std::vector<std::string>> Endpoint::send_getMessages(size_t startMsgId)
{
    EN_LOGI << "sending getMessages(startMsgId=" << startMsgId << ")";

    auto request = Request::GetMessages::create(startMsgId);
    auto context = OutcomingRequestContext::GetMessages::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context));

    return future;
}

boost::future<Timestamp_t> Endpoint::send_ping(Timestamp_t pingTimestamp)
{
    EN_LOGI << "sending ping(pingTimestamp=" << pingTimestamp << ")";

    auto request = Request::Ping::create(pingTimestamp);
    auto context = OutcomingRequestContext::Ping::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context));

    return future;
}

boost::future<void> Endpoint::send_secondaryNodeReady(std::string secondaryName)
{
    EN_LOGI << "sending secondaryNodeReady(secondaryName='" << secondaryName << "')";

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

                const auto msgId = request->messageId();
                auto       msg   = request->flushMessage();

                EN_LOGI << "incoming_addMessage(msgId=" << msgId << ", msg='" << msg << "')";

                incoming_addMessage(msgId, std::move(msg), promise);

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

                const auto startMsgId = request->startMessageId();

                EN_LOGI << "incoming_getMessages(startMsgId=" << startMsgId << ")";

                incoming_getMessages(startMsgId, promise);

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

                const auto pingTimestamp = request->timestamp();

                EN_LOGI << "incoming_ping(pingTimestamp=" << pingTimestamp << ")";

                incoming_ping(pingTimestamp, promise);

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

                auto secondaryNodeName = request->flushSecondaryNodeName();

                EN_LOGI << "incoing_secondaryNodeReady(secondaryNodeName='" << secondaryNodeName << "')";

                incoming_secondaryNodeReady(std::move(secondaryNodeName), promise);

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
