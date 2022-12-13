#include "proto/communicationendpoint.h"

#include <utility>

#include "incomingrequestsmanager.h"
#include "outcomingrequestsmanager.h"
#include "socketwrapper.h"

namespace Proto
{
std::shared_ptr<CommunicationEndpoint> CommunicationEndpoint::create(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    size_t                       sendTimeoutMs)
{
    auto res = std::shared_ptr<CommunicationEndpoint>{ new CommunicationEndpoint{
        ioContext, std::move(socket), sendTimeoutMs } };
    res->registerConnections();
    return res;
}

CommunicationEndpoint::~CommunicationEndpoint()
{
    socketWrapper_->invalidate();
    for (auto &connection : connections_)
        connection.disconnect();
}

void CommunicationEndpoint::run()
{
    socketWrapper_->run();
}

boost::future<Response::AddMessage>
    CommunicationEndpoint::send_addMessage(std::shared_ptr<const Request::AddMessage> request)
{
    return outcomingRequestsManager_->send(std::move(request));
}

boost::future<Response::GetMessages>
    CommunicationEndpoint::send_getMessages(std::shared_ptr<const Request::GetMessages> request)
{
    return outcomingRequestsManager_->send(std::move(request));
}

boost::future<Response::SecondaryNodeReady>
    CommunicationEndpoint::send_secondaryNodeReady(std::shared_ptr<const Request::SecondaryNodeReady> request)
{
    return outcomingRequestsManager_->send(std::move(request));
}

boost::future<Response::Pong> CommunicationEndpoint::send_ping(std::shared_ptr<const Request::Ping> request)
{
    return outcomingRequestsManager_->send(std::move(request));
}

CommunicationEndpoint::CommunicationEndpoint(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    size_t                       sendTimeoutMs)
    : socketWrapper_{ SocketWrapper::create(ioContext, std::move(socket)) },
      incomingRequestsManager_{ IncomingRequestsManager::create(socketWrapper_) },
      outcomingRequestsManager_{ OutcomingRequestsManager::create(socketWrapper_, sendTimeoutMs) }
{
}

template <Concepts::Request Request>
void CommunicationEndpoint::forwardIncomingRequestConnection(
    RequestSignal<Request> &source,
    RequestSignal<Request> &target)
{
    connections_.push_back(source.connect(
        [this, &target, weakSelf = weak_from_this()](
            std::shared_ptr<Request>                                                request,
            std::shared_ptr<boost::promise<typename Concepts::Response_t<Request>>> responsePromise)
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
                return;

            target(std::move(request), std::move(responsePromise));
        }));
}

void CommunicationEndpoint::registerConnections()
{
    connections_.push_back(socketWrapper_->invalidated.connect(
        [this, weakSelf = weak_from_this()]()
        {
            auto self = weakSelf.lock();

            if (self == nullptr)
                return;

            invalidated();
        }));

    // Proto: EXTENSION POINT
    forwardIncomingRequestConnection(incomingRequestsManager_->incoming_addMessage, incoming_addMessage);
    forwardIncomingRequestConnection(incomingRequestsManager_->incoming_getMessages, incoming_getMessages);
    forwardIncomingRequestConnection(
        incomingRequestsManager_->incoming_secondaryNodeReady, incoming_secondaryNodeReady);
    forwardIncomingRequestConnection(incomingRequestsManager_->incoming_ping, incoming_ping);
}

}    // namespace Proto
