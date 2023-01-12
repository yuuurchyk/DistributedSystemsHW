#pragma once

#include "utils/timestamp.h"

#include "outcomingrequestcontext/abstractoutcomingrequestcontext.h"

namespace Proto2::OutcomingRequestContext
{
class Ping final : public AbstractOutcomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<Ping> create();

    boost::future<Utils::Timestamp_t> get_future();

    void onResponseRecieved(boost::asio::const_buffer payload) override;
    void invalidate(InvalidationReason) override;

private:
    Ping() = default;

    boost::promise<Utils::Timestamp_t> promise_;
};

}    // namespace Proto2::OutcomingRequestContext
