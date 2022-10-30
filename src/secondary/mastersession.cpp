#include "mastersession.h"

#include <utility>
#include <vector>

#include "logger/logger.h"
#include "protocol/codes.h"
#include "protocol/request/getstrings.h"
#include "protocol/request/pushstring.h"
#include "protocol/request/request.h"
#include "protocol/response/getstrings.h"
#include "protocol/response/pushstring.h"
#include "protocol/response/response.h"

#include "messages.h"

using error_code = boost::system::error_code;
using namespace boost::asio;
using namespace protocol;

namespace
{
void handleRequest(std::weak_ptr<CommunicationEndpoint> endpoint,
                   request::Request                     request)
{
    if (!request.valid())
        return;

    switch (request.opCode())
    {
    case codes::OpCode::GET_STRINGS:
    {
        auto getStringsRequest = request::GetStrings{ std::move(request) };

        if (!getStringsRequest.valid())
            return;

        const auto s       = ::messages.size();
        auto       strings = std::vector<std::string_view>{};
        strings.reserve(s);

        for (auto i = size_t{}; i < s; ++i)
            strings.push_back(::messages[i]);

        auto response = response::GetStrings::answer(getStringsRequest, strings);

        auto endpointLock = endpoint.lock();
        if (endpointLock != nullptr)
            endpointLock->sendResponse(std::move(response));

        break;
    }
    case codes::OpCode::PUSH_STRING:
    {
        auto pushStringRequest = request::PushString{ std::move(request) };

        if (!pushStringRequest.valid())
            return;

        ::messages.push_back(std::string{ pushStringRequest.string() });

        auto response = response::PushString::answer(pushStringRequest);

        auto endpointLock = endpoint.lock();
        if (endpointLock != nullptr)
            endpointLock->sendResponse(std::move(response));

        break;
    }
    }
}
}    // namespace

std::shared_ptr<MasterSession>
    MasterSession::create(io_context    &masterCommunicationContext,
                          unsigned short port,
                          IOContextPool &workersPool)
{
    return std::shared_ptr<MasterSession>(
        new MasterSession{ masterCommunicationContext, port, workersPool });
}

void MasterSession::run()
{
    acceptConnection();
}

MasterSession::MasterSession(io_context    &masterCommunicationContext,
                             unsigned short port,
                             IOContextPool &workersPool)
    : context_{ masterCommunicationContext },
      acceptor_{ context_, ip::tcp::endpoint{ ip::tcp::v4(), port } },
      workersPool_{ workersPool }
{
}

void MasterSession::acceptConnection()
{
    auto  socketOwner = std::make_unique<ip::tcp::socket>(context_);
    auto &socket      = *socketOwner;

    LOGI << "waiting for connection with master node";
    acceptor_.async_accept(
        socket,
        [this, self = shared_from_this(), socketOwner = std::move(socketOwner)](
            const error_code &error)
        {
            if (error)
            {
                LOGE << "Failed to accept socket, retrying";
                acceptConnection();
            }
            else
            {
                LOGI
                    << "Accepted connection with master, initiating endpoint communication";
                createCommunicationEndpoint(std::move(*socketOwner));
            }
        });
}

void MasterSession::createCommunicationEndpoint(ip::tcp::socket socket)
{
    LOGI << "creating communication endpoint";

    auto invalidationCallback =
        [this, self = weak_from_this()](std::shared_ptr<CommunicationEndpoint>)
    {
        auto selfLock = self.lock();
        if (selfLock != nullptr)
            selfLock->handleCommunicationEndpointInvalidation();
    };

    auto requestCallback =
        [this, self = weak_from_this()](std::shared_ptr<CommunicationEndpoint> endpoint,
                                        request::Request                       request)
    {
        auto selfLock = self.lock();

        if (selfLock == nullptr)
            return;

        LOGI << "Request from master recieved: " << request << ", pushing it to worker";

        auto a = [endpoint = std::move(endpoint), request = std::move(request)]()
        { LOGI << "HELLO?"; };

        workersPool_.getNext().post(
            [endpoint = std::move(endpoint),
             request  = std::make_shared<request::Request>(std::move(request))]
            { ::handleRequest(std::move(endpoint), std::move(*request)); });
    };

    communicationEndpoint_ =
        CommunicationEndpoint::create(context_,
                                      std::move(socket),
                                      std::move(requestCallback),
                                      std::move(invalidationCallback));
}

void MasterSession::handleCommunicationEndpointInvalidation()
{
    LOGW << "communication with master node dropped";
    communicationEndpoint_ = nullptr;
    acceptConnection();
}
