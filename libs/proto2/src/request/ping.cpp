#include "proto2/request/ping.h"

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto2::Request
{
std::shared_ptr<Ping> Ping::create(Timestamp_t timestamp)
{
    return std::shared_ptr<Ping>{ new Ping{ timestamp } };
}

std::shared_ptr<Ping> Ping::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    auto optTimestamp = deserializer.deserialize<Timestamp_t>();

    if (!optTimestamp.has_value() || !deserializer.atEnd())
        return {};

    return create(optTimestamp.value());
}

void Ping::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serialize(&timestamp_);
}

Timestamp_t Ping::timestamp() const noexcept
{
    return timestamp_;
}

const OpCode &Ping::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::PING_PONG };
    return kOpCode;
}

Ping::Ping(Timestamp_t timestamp) : timestamp_{ timestamp } {}

}    // namespace Proto2::Request
