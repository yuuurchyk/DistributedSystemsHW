#include "proto2/request/ping.h"

#include "serialization/buffersequenceserializer.h"

namespace Proto2::Request
{
std::unique_ptr<Ping> Ping::create(Timestamp_t timestamp)
{
    return std::unique_ptr<Ping>{ new Ping{ timestamp } };
}

std::unique_ptr<Ping> Ping::fromPayload(boost::asio::const_buffer buffer)
{
    if (buffer.size() != sizeof(Timestamp_t) || buffer.data() == nullptr)
        return {};

    auto timestamp = *reinterpret_cast<const Timestamp_t *>(buffer.data());
    return create(timestamp);
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

void Ping::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serialize(&timestamp_);
}

Ping::Ping(Timestamp_t timestamp) : timestamp_{ timestamp } {}

}    // namespace Proto2::Request
