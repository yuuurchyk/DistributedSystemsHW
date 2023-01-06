#include "proto2/response/addmessage.h"

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto2::Response
{

std::shared_ptr<AddMessage> AddMessage::create(Status status)
{
    return std::shared_ptr<AddMessage>{ new AddMessage{ status } };
}

std::shared_ptr<AddMessage> AddMessage::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    const auto optStatus = deserializer.deserialize<Status>();

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
        case Status::OK:
        case Status::NOT_ALLOWED:
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

    serializer.serialize(&status_);
}

auto AddMessage::status() const -> Status
{
    return status_;
}

const OpCode &AddMessage::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::ADD_MESSAGE };
    return kOpCode;
}

AddMessage::AddMessage(Status status) : status_{ status } {}

}    // namespace Proto2::Response
