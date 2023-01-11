#pragma once

#include "incomingrequestcontext/abstractincomingrequestcontext.h"

namespace Proto2::IncomingRequestContext
{
class SecondaryNodeReady final : public AbstractIncomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<SecondaryNodeReady> create(boost::asio::io_context &);

    boost::promise<void> flushPromise();

private:
    using AbstractIncomingRequestContext::AbstractIncomingRequestContext;

    void connectPromise() override;

    boost::promise<void> promise_;
};

}    // namespace Proto2::IncomingRequestContext
