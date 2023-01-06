#pragma once

#include <string>

#include "request/abstractrequest.h"

namespace Proto2::Request
{
class SecondaryNodeReady final : public AbstractRequest
{
public:
    [[nodiscard]] static std::shared_ptr<SecondaryNodeReady> create(std::string secondaryNodeName);

    [[nodiscard]] static std::shared_ptr<SecondaryNodeReady> fromPayload(boost::asio::const_buffer);
    void serializePayload(std::vector<boost::asio::const_buffer> &) const override;

    const std::string &secondaryNodeName() const noexcept;
    std::string        flushSecondaryNodeName();

    const OpCode &opCode() const override;

private:
    SecondaryNodeReady(std::string secondaryNodeName);

    std::string secondaryNodeName_;
};

}    // namespace Proto2::Request
