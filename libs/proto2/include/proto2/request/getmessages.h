#pragma once

#include "abstractrequest.h"

namespace Proto2::Request
{
class GetMessages final : public AbstractRequest
{
public:
    [[nodiscard]] static std::shared_ptr<GetMessages> create(size_t startMessageId = 0);

    [[nodiscard]] static std::shared_ptr<GetMessages> fromPayload(boost::asio::const_buffer);
    void serializePayload(std::vector<boost::asio::const_buffer> &) const override;

    size_t startMessageId() const noexcept;

    const OpCode &opCode() const override;

private:
    GetMessages(size_t startMessageId);

    const size_t startMessageId_;
};

}    // namespace Proto2::Request
