#pragma once

#include "utils/timestamp.h"

#include "incomingrequestcontext/abstractincomingrequestcontext.h"

namespace Proto2::IncomingRequestContext
{
class Ping final : public AbstractIncomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<Ping> create(boost::asio::io_context &executionContext);

    boost::promise<Utils::Timestamp_t> flushPromise();

private:
    using AbstractIncomingRequestContext::AbstractIncomingRequestContext;

    void connectPromise() override;

    boost::promise<Utils::Timestamp_t> promise_{};
};

}    // namespace Proto2::IncomingRequestContext
