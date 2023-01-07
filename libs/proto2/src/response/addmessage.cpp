#include "response/addmessage.h"

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto2::Response
{

std::shared_ptr<AddMessage> AddMessage::create(AddMessageStatus status)
{
    return std::shared_ptr<AddMessage>{ new AddMessage{ status } };
}

std::shared_ptr<AddMessage> AddMessage::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    const auto optStatus = deserializer.deserialize<AddMessageStatus>();

    if (!optStatus.has_value() || !deserializer.atEnd())
    {
        return {};
    }
    else
    {
        // check status enum value
        const auto status   = optStatus.value();
        auto       statusOk = false;

        switch (status)
        {
        case AddMessageStatus::OK:
        case AddMessageStatus::NOT_ALLOWED:
            statusOk = true;
            break;
        }

        if (!statusOk)
            return {};
        else
            return create(status);
    }
}

void AddMessage::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serializeWoOwnership(status_);
}

AddMessageStatus AddMessage::status() const
{
    return status_;
}

const OpCode &AddMessage::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::ADD_MESSAGE };
    return kOpCode;
}

AddMessage::AddMessage(AddMessageStatus status) : status_{ status } {}

}    // namespace Proto2::Response
