#pragma once

#include "proto2/timestamp.h"

#include "abstractrequest.h"

namespace Proto2::Request
{
class Ping final : public AbstractRequest
{
public:
    [[nodiscard]] static std::unique_ptr<Ping> create(Timestamp_t);
    [[nodiscard]] static std::unique_ptr<Ping> fromPayload(boost::asio::const_buffer);

    Timestamp_t timestamp() const noexcept;

    const OpCode &opCode() const override;
    void          serializePayload(std::vector<boost::asio::const_buffer> &) const override;

private:
    Ping(Timestamp_t);

    const Timestamp_t timestamp_;
};

}    // namespace Proto2::Request
