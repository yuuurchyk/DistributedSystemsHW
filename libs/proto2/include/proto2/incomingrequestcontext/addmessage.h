#pragma once

#include "abstractincomingrequestcontext.h"
#include "proto2/response/addmessage.h"

namespace Proto2::IncomingRequestContext
{
class AddMessage final : public AbstractIncomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<AddMessage> create(boost::asio::io_context &);

    boost::promise<Response::AddMessage::Status> flushPromise();

private:
    using AbstractIncomingRequestContext::AbstractIncomingRequestContext;

    void connectPromise() override;

    boost::promise<Response::AddMessage::Status> promise_{};
};

}    // namespace Proto2::IncomingRequestContext
