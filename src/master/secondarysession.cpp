#include "secondarysession.h"

#include <utility>

using error_code = boost::system::error_code;
using namespace boost::asio;

std::shared_ptr<SecondarySession>
    SecondarySession::create(boost::asio::io_context       &secondaryCommunicationContext,
                             boost::asio::ip::tcp::endpoint endpoint,
                             boost::posix_time::milliseconds reconnectTimeout)
{
    return std::shared_ptr<SecondarySession>{ new SecondarySession{
        secondaryCommunicationContext,
        std::move(endpoint),
        std::move(reconnectTimeout) } };
}

void SecondarySession::run()
{
    connect();
}

size_t SecondarySession::getNextRequestId()
{
    std::shared_lock<std::shared_mutex> lck{ communicationEndpointMutex_ };
    if (communicationEndpoint_ == nullptr)
        return 0;
    else
        return communicationEndpoint_->getNextRequestId();
}

void SecondarySession::sendRequest(protocol::request::Request request,
                                   response_callback_fn       responseCallback)
{
    if (!request.valid())
        return;

    auto communicationEndpointCallback =
        [callback = std::move(responseCallback)](
            std::shared_ptr<CommunicationEndpoint>,
            std::optional<protocol::response::Response> response,
            protocol::request::Request                  request)
    {
        if (callback != nullptr)
            callback(std::move(response), std::move(request));
    };

    std::shared_lock<std::shared_mutex> lck{ communicationEndpointMutex_ };
    if (communicationEndpoint_ == nullptr)
    {
        ID_LOGW
            << "communication endpoint with secondary node is offline, responsing right away";
        communicationEndpointCallback(nullptr, /*response*/ {}, std::move(request));
    }
    else
    {
        ID_LOGI << "sending request";
        communicationEndpoint_->sendRequest(std::move(request),
                                            std::move(communicationEndpointCallback));
    }
}

SecondarySession::SecondarySession(boost::asio::io_context        &context,
                                   boost::asio::ip::tcp::endpoint  endpoint,
                                   boost::posix_time::milliseconds reconnectTimeout)
    : context_{ context },
      endpoint_{ std::move(endpoint) },
      reconnectTimeout_{ std::move(reconnectTimeout) },
      reconnectTimer_{ context }
{
}

void SecondarySession::connect()
{
    ID_LOGI << "connecting...";

    auto  socketOwner = std::make_unique<ip::tcp::socket>(context_);
    auto &socket      = *socketOwner;

    socket.async_connect(endpoint_,
                         [this,
                          self        = shared_from_this(),
                          socketOwner = std::move(socketOwner)](const error_code &error)
                         {
                             if (error)
                             {
                                 ID_LOGW << "failed to connect";
                                 scheduleReconnect();
                             }
                             else
                             {
                                 createCommunicationEndpoint(std::move(*socketOwner));
                             }
                         });
}

void SecondarySession::scheduleReconnect()
{
    ID_LOGW << "scheduling reconnect";

    reconnectTimer_.expires_from_now(reconnectTimeout_);

    reconnectTimer_.async_wait(
        [this, self = shared_from_this()](const error_code &error)
        {
            if (!error)
                connect();
        });
}

void SecondarySession::createCommunicationEndpoint(ip::tcp::socket socket)
{
    ID_LOGI << "creating communication endpoint";

    auto invalidationCallback =
        [this, self = shared_from_this()](std::shared_ptr<CommunicationEndpoint>)
    { handleCommunicationEndpointInvalidation(); };

    std::unique_lock<std::shared_mutex> lck{ communicationEndpointMutex_ };
    communicationEndpoint_ =
        CommunicationEndpoint::create(context_,
                                      std::move(socket),
                                      /*request_callback_fn*/ nullptr,
                                      std::move(invalidationCallback));
    communicationEndpoint_->run();
    // note: we don't react to requests from secondary node
    // TODO: rethink (e.g. if secondary node restores its state, it might ask for data)
}

void SecondarySession::handleCommunicationEndpointInvalidation()
{
    ID_LOGI << "connection to secondary node dropped";

    {
        std::unique_lock<std::shared_mutex> lck{ communicationEndpointMutex_ };
        communicationEndpoint_ = nullptr;
    }

    scheduleReconnect();
}
