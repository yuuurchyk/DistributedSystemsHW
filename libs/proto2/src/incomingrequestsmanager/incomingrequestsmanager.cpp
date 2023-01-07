#include "incomingrequestsmanager/incomingrequestsmanager.h"

#include <stdexcept>
#include <utility>

#include "net-utils/thenpost.h"

#include "response/abstractresponse.h"

namespace Proto2
{
IncomingRequestsManager::Pending::Pending(
    size_t                                                                  id,
    std::shared_ptr<Request::AbstractRequest>                               request,
    std::shared_ptr<IncomingRequestContext::AbstractIncomingRequestContext> context)
    : id{ id }, request{ std::move(request) }, context{ std::move(context) }
{
}

std::shared_ptr<IncomingRequestsManager> IncomingRequestsManager::create(std::shared_ptr<SocketWrapper> socketWrapper)
{
    if (socketWrapper == nullptr)
    {
        LOGE << "socket wrapper cannot be nullptr";
        throw std::logic_error{ "socket wrapper cannot be nullptr" };
    }

    return std::shared_ptr<IncomingRequestsManager>{ new IncomingRequestsManager{ std::move(socketWrapper) } };
}

void IncomingRequestsManager::addIncomingRequest(
    size_t                                                                  id,
    std::shared_ptr<Request::AbstractRequest>                               request,
    std::shared_ptr<IncomingRequestContext::AbstractIncomingRequestContext> context)
{
    if (request == nullptr)
    {
        EN_LOGW << "request cannot be nullptr, not adding";
        return;
    }
    if (context == nullptr)
    {
        EN_LOGW << "request context cannot be nullptr, not adding";
        return;
    }

    auto pending = Pending{ id, std::move(request), std::move(context) };
    pending.context->responseRecieved.connect([id](std::shared_ptr<Response::AbstractResponse> response)
                                              { static_assert(false); });
}

}    // namespace Proto2
