#include "request/getmessages.h"

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto2::Request
{
std::shared_ptr<GetMessages> GetMessages::create(size_t startMessageId)
{
    return std::shared_ptr<GetMessages>{ new GetMessages{ startMessageId } };
}

std::shared_ptr<GetMessages> GetMessages::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    auto optStartMessageId = deserializer.deserialize<size_t>();

    if (!optStartMessageId.has_value() || !deserializer.atEnd())
        return {};

    return create(optStartMessageId.value());
}

void GetMessages::serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serializeWoOwnership(startMessageId_);
}

size_t GetMessages::startMessageId() const noexcept
{
    return startMessageId_;
}

const OpCode &GetMessages::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::GET_MESSAGES };
    return kOpCode;
}

GetMessages::GetMessages(size_t startMessageId) : startMessageId_{ startMessageId } {}

}    // namespace Proto2::Request
