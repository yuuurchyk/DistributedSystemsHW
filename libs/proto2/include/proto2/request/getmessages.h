#pragma once

#include "abstractrequest.h"

namespace Proto2::Request
{
class GetMessages final : public AbstractRequest
{
public:
    [[nodiscard]] static std::unique_ptr<GetMessages> create(size_t startMessageId = 0);
    [[nodiscard]] static std::unique_ptr<GetMessages> fromPayload(boost::asio::const_buffer);

    size_t startMessageId() const noexcept;

    const OpCode &opCode() const override;
    void          serializePayload(std::vector<boost::asio::const_buffer> &) const override;

private:
    GetMessages(size_t startMessageId);

    const size_t startMessageId_;
};

}    // namespace Proto2::Request