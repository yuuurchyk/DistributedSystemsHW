#include "proto2/endpoint.h"

#include <functional>
#include <random>
#include <thread>

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

    impl_t(
        std::string                                                 endpointId,
        boost::asio::io_context                                    &ioContext,
        boost::asio::ip::tcp::socket                                socket,
        duration_milliseconds_t                                     responseTimeout,
        std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds)
        : ioContext{ ioContext },
          socketWrapper{ SocketWrapper::create(endpointId, ioContext, std::move(socket)) },
          incomingRequestsManager{ IncomingRequestsManager::create(endpointId, socketWrapper) },
          outcomingRequestsManager{ OutcomingRequestsManager::create(endpointId, socketWrapper, responseTimeout) },
          sendDelayDistribution_{ artificialSendDelayBounds.first.count(), artificialSendDelayBounds.second.count() },
          artificialSendDelayBounds_{ std::move(artificialSendDelayBounds) }
    {
    }

    // thread safe
    duration_milliseconds_t chooseSendDelay()
    {
        auto distribution = std::uniform_int_distribution<size_t>{ artificialSendDelayBounds_.first.count(),
                                                                   artificialSendDelayBounds_.second.count() };
        return duration_milliseconds_t{ distribution(engine_) };
    }

    boost::asio::io_context                  &ioContext;
    std::shared_ptr<SocketWrapper>            socketWrapper;
    std::shared_ptr<IncomingRequestsManager>  incomingRequestsManager;
    std::shared_ptr<OutcomingRequestsManager> outcomingRequestsManager;

private:
    static thread_local std::seed_seq                                 engineSeq_;
    static thread_local std::mt19937                                  engine_;
    const std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds_;

    std::uniform_int_distribution<size_t> sendDelayDistribution_;
};

thread_local std::seed_seq Endpoint::impl_t::engineSeq_{ std::hash<std::thread::id>{}(std::this_thread::get_id()),
                                                         size_t{ 47 } };
thread_local std::mt19937  Endpoint::impl_t::engine_{ engineSeq_ };

std::shared_ptr<Endpoint> Endpoint::create(
    std::string                                                 id,
    boost::asio::io_context                                    &ioContext,
    boost::asio::ip::tcp::socket                                socket,
    duration_milliseconds_t                                     outcomingRequestTimeout,
    std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds)
{
    if (artificialSendDelayBounds.first.count() > artificialSendDelayBounds.second.count())
    {
        LOGW << "Passed wrong artificialSendDelayBounds when constructing Endpoint, fixing up";
        std::swap(artificialSendDelayBounds.first, artificialSendDelayBounds.second);
    }

    auto self = std::shared_ptr<Endpoint>{ new Endpoint{
        std::move(id), ioContext, std::move(socket), outcomingRequestTimeout, std::move(artificialSendDelayBounds) } };

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
    const auto artificialDelay = impl().chooseSendDelay();

    EN_LOGI << "sending addMessage(msgId=" << msgId << ", msg='" << msg << "') (artificially delayed by "
            << artificialDelay.count() << "ms)";

    auto request = Request::AddMessage::create(msgId, msg);
    auto context = OutcomingRequestContext::AddMessage::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context), artificialDelay);

    return future;
}

boost::future<std::vector<std::string>> Endpoint::send_getMessages(size_t startMsgId)
{
    const auto artificialDelay = impl().chooseSendDelay();

    EN_LOGI << "sending getMessages(startMsgId=" << startMsgId << ") (artificially delayed by "
            << artificialDelay.count() << "ms)";

    auto request = Request::GetMessages::create(startMsgId);
    auto context = OutcomingRequestContext::GetMessages::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context), artificialDelay);

    return future;
}

boost::future<Timestamp_t> Endpoint::send_ping(Timestamp_t pingTimestamp)
{
    const auto artificialDelay = impl().chooseSendDelay();

    EN_LOGI << "sending ping(pingTimestamp=" << pingTimestamp << ") (artificially delayed by "
            << artificialDelay.count() << "ms)";

    auto request = Request::Ping::create(pingTimestamp);
    auto context = OutcomingRequestContext::Ping::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context), artificialDelay);

    return future;
}

boost::future<void> Endpoint::send_secondaryNodeReady(std::string secondaryName)
{
    const auto artificialDelay = impl().chooseSendDelay();

    EN_LOGI << "sending secondaryNodeReady(secondaryName='" << secondaryName << "') (artificially delayed by "
            << artificialDelay.count() << "ms)";

    auto request = Request::SecondaryNodeReady::create(std::move(secondaryName));
    auto context = OutcomingRequestContext::SecondaryNodeReady::create();

    auto future = context->future();

    impl().outcomingRequestsManager->sendRequest(std::move(request), std::move(context), artificialDelay);

    return future;
}

Endpoint::Endpoint(
    std::string                                                 id,
    boost::asio::io_context                                    &ioContext,
    boost::asio::ip::tcp::socket                                socket,
    duration_milliseconds_t                                     responseTimeout,
    std::pair<duration_milliseconds_t, duration_milliseconds_t> artificialSendDelayBounds)
    : StringIdEntity<Endpoint>{ id },
      TW5VNznK_{ std::make_unique<impl_t>(
          id,
          ioContext,
          std::move(socket),
          responseTimeout,
          std::move(artificialSendDelayBounds)) }
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

                EN_LOGI << "incoming_secondaryNodeReady(secondaryNodeName='" << secondaryNodeName << "')";

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
