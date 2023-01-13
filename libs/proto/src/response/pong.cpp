#include "response/pong.h"

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto::Response
{

std::shared_ptr<Pong> Pong::create(Utils::Timestamp_t timestamp)
{
    return std::shared_ptr<Pong>{ new Pong{ timestamp } };
}

std::shared_ptr<Pong> Pong::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    const auto optTimestamp = deserializer.deserialize<Utils::Timestamp_t>();

    if (!optTimestamp.has_value() || !deserializer.atEnd())
        return {};
    else
        return create(optTimestamp.value());
}

void Pong::serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serializeWoOwnership(timestamp_);
}

Utils::Timestamp_t Pong::timestamp() const
{
    return timestamp_;
}

const OpCode &Pong::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::PING_PONG };
    return kOpCode;
}

Pong::Pong(Utils::Timestamp_t timestamp) : timestamp_{ timestamp } {}

}    // namespace Proto::Response
