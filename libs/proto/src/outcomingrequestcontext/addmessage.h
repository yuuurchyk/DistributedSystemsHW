#pragma once

#include "outcomingrequestcontext/abstractoutcomingrequestcontext.h"
#include "proto/enums.h"

namespace Proto::OutcomingRequestContext
{
class AddMessage final : public AbstractOutcomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<AddMessage> create();

    boost::future<AddMessageStatus> get_future();

    void onResponseRecieved(boost::asio::const_buffer payload) override;
    void invalidate(InvalidationReason) override;

private:
    AddMessage() = default;

    boost::promise<AddMessageStatus> promise_;
};

}    // namespace Proto::OutcomingRequestContext
