#pragma once

#include "proto/enums.h"
#include "response/abstractresponse.h"

namespace Proto::Response
{
class AddMessage final : public AbstractResponse
{
public:
    [[nodiscard]] static std::shared_ptr<AddMessage> create(AddMessageStatus);

    [[nodiscard]] static std::shared_ptr<AddMessage> fromPayload(boost::asio::const_buffer);
    void serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &seq) const override;

    AddMessageStatus status() const;

    const OpCode &opCode() const override;

private:
    AddMessage(AddMessageStatus);

    const AddMessageStatus status_;
};

}    // namespace Proto::Response
