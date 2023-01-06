#pragma once

#include "abstractoutcomingrequestcontext.h"
#include "proto2/response/addmessage.h"

namespace Proto2::OutcomingRequestContext
{
class AddMessage final : public AbstractOutcomingRequestContext
{
public:
    [[nodiscard]] static std::unique_ptr<AddMessage> create();

    boost::future<Response::AddMessage::Status> future();

    void onResponseRecieved(boost::asio::const_buffer payload) override;
    void invalidate(InvalidationReason) override;

private:
    AddMessage() = default;

    boost::promise<Response::AddMessage::Status> promise_;
};

}    // namespace Proto2::OutcomingRequestContext
