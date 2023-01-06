#pragma once

#include "proto2/incomingrequestcontext/abstractincomingrequestcontext.h"
#include "proto2/timestamp.h"

namespace Proto2::IncomingRequestContext
{
class Ping final : public AbstractIncomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<Ping> create(boost::asio::io_context &);

    boost::promise<Timestamp_t> flushPromise();

private:
    using AbstractIncomingRequestContext::AbstractIncomingRequestContext;

    void connectPromise() override;

    boost::promise<Timestamp_t> promise_{};
};

}    // namespace Proto2::IncomingRequestContext
