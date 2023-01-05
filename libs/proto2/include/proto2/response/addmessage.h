#pragma once

#include <cstdint>

#include "abstractresponse.h"

namespace Proto2::Response
{

class AddMessage final : public AbstractResponse
{
public:
    enum class Status : uint8_t
    {
        OK = 0,
        NOT_ALLOWED
    };

    [[nodiscard]] static std::unique_ptr<AddMessage> create(Status);

    [[nodiscard]] static std::unique_ptr<AddMessage> fromPayload(boost::asio::const_buffer);
    void serializePayload(std::vector<boost::asio::const_buffer> &seq) const override;

    Status status() const;

    const OpCode &opCode() const override;

private:
    AddMessage(Status);

    const Status status_;
};

}    // namespace Proto2::Response
