#pragma once

#include <string>
#include <vector>

#include "abstractoutcomingrequestcontext.h"

namespace Proto2::OutcomingRequestContext
{
class GetMessages final : public AbstractOutcomingRequestContext
{
public:
    [[nodiscard]] static std::unique_ptr<GetMessages> create();

    boost::future<std::vector<std::string>> future();

    void onResponseRecieved(boost::asio::const_buffer payload) override;
    void invalidate(InvalidationReason) override;

private:
    GetMessages() = default;

    boost::promise<std::vector<std::string>> promise_;
};

}    // namespace Proto2::OutcomingRequestContext
