#pragma once

#include "proto2/outcomingrequestcontext/abstractoutcomingrequestcontext.h"
#include "proto2/timestamp.h"

namespace Proto2::OutcomingRequestContext
{
class Ping final : public AbstractOutcomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<Ping> create();

    boost::future<Timestamp_t> future();

    void onResponseRecieved(boost::asio::const_buffer payload) override;
    void invalidate(InvalidationReason) override;

private:
    Ping() = default;

    boost::promise<Timestamp_t> promise_;
};

}    // namespace Proto2::OutcomingRequestContext
