#pragma once

#include "response/abstractresponse.h"

namespace Proto2::Response
{
class SecondaryNodeReady final : public AbstractResponse
{
public:
    [[nodiscard]] static std::shared_ptr<SecondaryNodeReady> create();

    [[nodiscard]] static std::shared_ptr<SecondaryNodeReady> fromPayload(boost::asio::const_buffer);
    void serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &) const override;

    const OpCode &opCode() const override;

private:
    SecondaryNodeReady() = default;
};

}    // namespace Proto2::Response
