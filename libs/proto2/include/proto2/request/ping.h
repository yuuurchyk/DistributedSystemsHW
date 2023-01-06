#pragma once

#include "proto2/request/abstractrequest.h"
#include "proto2/timestamp.h"

namespace Proto2::Request
{
class Ping final : public AbstractRequest
{
public:
    [[nodiscard]] static std::shared_ptr<Ping> create(Timestamp_t);

    [[nodiscard]] static std::shared_ptr<Ping> fromPayload(boost::asio::const_buffer);
    void serializePayload(std::vector<boost::asio::const_buffer> &) const override;

    Timestamp_t timestamp() const noexcept;

    const OpCode &opCode() const override;

private:
    Ping(Timestamp_t);

    const Timestamp_t timestamp_;
};

}    // namespace Proto2::Request
