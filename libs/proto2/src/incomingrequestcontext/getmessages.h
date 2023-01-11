#pragma once

#include <string_view>
#include <vector>

#include "incomingrequestcontext/abstractincomingrequestcontext.h"

namespace Proto2::IncomingRequestContext
{
class GetMessages final : public AbstractIncomingRequestContext
{
public:
    [[nodiscard]] static std::shared_ptr<GetMessages> create(boost::asio::io_context &executionContext);

    boost::promise<std::vector<std::string_view>> flushPromise();

private:
    using AbstractIncomingRequestContext::AbstractIncomingRequestContext;

    void connectPromise() override;

    boost::promise<std::vector<std::string_view>> promise_{};
};

}    // namespace Proto2::IncomingRequestContext
