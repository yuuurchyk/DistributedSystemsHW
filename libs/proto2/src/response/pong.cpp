#include "proto2/response/pong.h"

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto2::Response
{

std::unique_ptr<Pong> Pong::create(Timestamp_t timestamp)
{
    return std::unique_ptr<Pong>{ new Pong{ timestamp } };
}

std::unique_ptr<Pong> Pong::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    const auto optTimestamp = deserializer.deserialize<Timestamp_t>();

    if (!optTimestamp.has_value() || !deserializer.atEnd())
        return {};
    else
        return create(optTimestamp.value());
}

void Pong::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serialize(&timestamp_);
}

Timestamp_t Pong::timestamp() const
{
    return timestamp_;
}

const OpCode &Pong::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::PING_PONG };
    return kOpCode;
}

Pong::Pong(Timestamp_t timestamp) : timestamp_{ timestamp } {}

}    // namespace Proto2::Response
