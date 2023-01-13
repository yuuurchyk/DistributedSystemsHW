#pragma once

#include "utils/timestamp.h"

#include "request/abstractrequest.h"

namespace Proto::Request
{
class Ping final : public AbstractRequest
{
public:
    [[nodiscard]] static std::shared_ptr<Ping> create(Utils::Timestamp_t);

    [[nodiscard]] static std::shared_ptr<Ping> fromPayload(boost::asio::const_buffer);
    void serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &) const override;

    Utils::Timestamp_t timestamp() const noexcept;

    const OpCode &opCode() const override;

private:
    Ping(Utils::Timestamp_t);

    const Utils::Timestamp_t timestamp_;
};

}    // namespace Proto::Request
