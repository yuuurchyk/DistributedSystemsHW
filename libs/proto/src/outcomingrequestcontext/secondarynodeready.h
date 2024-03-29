#pragma once

#include "outcomingrequestcontext/abstractoutcomingrequestcontext.h"

namespace Proto::OutcomingRequestContext
{
class SecondaryNodeReady final : public AbstractOutcomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<SecondaryNodeReady> create();

    boost::future<void> get_future();

    void onResponseRecieved(boost::asio::const_buffer payload) override;
    void invalidate(InvalidationReason) override;

private:
    SecondaryNodeReady() = default;

    boost::promise<void> promise_;
};

}    // namespace Proto::OutcomingRequestContext
