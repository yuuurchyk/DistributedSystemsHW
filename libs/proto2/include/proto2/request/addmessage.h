#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "abstractrequest.h"

namespace Proto2::Request
{
class AddMessage final : public AbstractRequest
{
public:
    [[nodiscard]] static std::shared_ptr<AddMessage> create(size_t messageId, std::string_view messageView);

    [[nodiscard]] static std::shared_ptr<AddMessage> fromPayload(boost::asio::const_buffer);
    void serializePayload(std::vector<boost::asio::const_buffer> &) const override;

    size_t           messageId() const noexcept;
    std::string_view messageView() const;
    // in case fromPayload() was used, retrieve the read message
    std::string flushMessage();

    const OpCode &opCode() const override;

private:
    AddMessage(size_t messageId, std::string_view messageView);
    AddMessage(size_t messageId, std::string message);

    const size_t messageId_;

    // in case fromPayload() was used
    std::optional<std::string> messageOwner_;

    const std::string_view messageView_;
};

}    // namespace Proto2::Request
