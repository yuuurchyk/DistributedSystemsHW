#pragma once

#include "incomingrequestcontext/abstractincomingrequestcontext.h"

namespace Proto::IncomingRequestContext
{
class SecondaryNodeReady final : public AbstractIncomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<SecondaryNodeReady> create(boost::asio::io_context &executionContext);

    boost::promise<void> flushPromise();

private:
    using AbstractIncomingRequestContext::AbstractIncomingRequestContext;

    void connectPromise() override;

    boost::promise<void> promise_;
};

}    // namespace Proto::IncomingRequestContext
