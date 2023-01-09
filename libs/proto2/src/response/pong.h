#pragma once

#include "proto2/timestamp.h"
#include "response/abstractresponse.h"

namespace Proto2::Response
{
class Pong : public AbstractResponse
{
public:
    [[nodiscard]] static std::shared_ptr<Pong> create(Timestamp_t);

    [[nodiscard]] static std::shared_ptr<Pong> fromPayload(boost::asio::const_buffer);
    void serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &) const override;

    Timestamp_t timestamp() const;

    const OpCode &opCode() const override;

private:
    Pong(Timestamp_t timestamp);

    const Timestamp_t timestamp_{};
};

}    // namespace Proto2::Response
