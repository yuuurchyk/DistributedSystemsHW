#pragma once

#include "proto2/enums.h"
#include "proto2/outcomingrequestcontext/abstractoutcomingrequestcontext.h"

namespace Proto2::OutcomingRequestContext
{
class AddMessage final : public AbstractOutcomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<AddMessage> create();

    boost::future<AddMessageStatus> future();

    void onResponseRecieved(boost::asio::const_buffer payload) override;
    void invalidate(InvalidationReason) override;

private:
    AddMessage() = default;

    boost::promise<AddMessageStatus> promise_;
};

}    // namespace Proto2::OutcomingRequestContext
