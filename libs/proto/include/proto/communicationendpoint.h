#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "proto/concepts.h"
#include "utils/copymove.h"

namespace Proto
{
class SocketWrapper;
class IncomingRequestsManager;
class OutcomingRequestsManager;

class CommunicationEndpoint : public std::enable_shared_from_this<CommunicationEndpoint>,
                              private logger::Entity<CommunicationEndpoint>
{
    DISABLE_COPY_MOVE(CommunicationEndpoint)
public:
    template <typename T>
    using signal = boost::signals2::signal<T>;
    template <Concepts::Request Request>
    using RequestSignal =
        signal<void(std::shared_ptr<Request>, std::shared_ptr<boost::promise<typename Concepts::Response_t<Request>>>)>;

    /**
     * @param ioContext - io context, associated with @p socket
     * @param socket
     * @param sendTimeoutMs
     */
    [[nodiscard]] static std::shared_ptr<CommunicationEndpoint>
        create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket, size_t sendTimeoutMs);

    ~CommunicationEndpoint();

    void run();

    boost::future<Response::AddMessage>  send_addMessage(std::shared_ptr<const Request::AddMessage>);
    boost::future<Response::GetMessages> send_getMessages(std::shared_ptr<const Request::GetMessages>);

    RequestSignal<Request::AddMessage>  incoming_addMessage;
    RequestSignal<Request::GetMessages> incoming_getMessages;

    signal<void()> invalidated;

private:
    template <Concepts::Request Request>
    void forwardIncomingRequestConnection(RequestSignal<Request> &source, RequestSignal<Request> &target);

    CommunicationEndpoint(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket, size_t sendTimeoutMs);

    void registerConnections();

    std::shared_ptr<SocketWrapper>            socketWrapper_;
    std::shared_ptr<IncomingRequestsManager>  incomingRequestsManager_;
    std::shared_ptr<OutcomingRequestsManager> outcomingRequestsManager_;

    std::vector<boost::signals2::connection> connections_;
};

}    // namespace Proto
