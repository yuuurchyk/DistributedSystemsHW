#pragma once

#include "incomingrequestcontext/abstractincomingrequestcontext.h"
#include "proto2/enums.h"

namespace Proto2::IncomingRequestContext
{
class AddMessage final : public AbstractIncomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<AddMessage> create(boost::asio::io_context &executionContext);

    boost::promise<AddMessageStatus> flushPromise();

private:
    using AbstractIncomingRequestContext::AbstractIncomingRequestContext;

    void connectPromise() override;

    boost::promise<AddMessageStatus> promise_{};
};

}    // namespace Proto2::IncomingRequestContext
