#include "proto2/response/secondarynodeready.h"

#include "deserialization/bufferdeserializer.h"

namespace Proto2::Response
{

std::shared_ptr<SecondaryNodeReady> SecondaryNodeReady::create()
{
    return std::shared_ptr<SecondaryNodeReady>{ new SecondaryNodeReady{} };
}

std::shared_ptr<SecondaryNodeReady> SecondaryNodeReady::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    if (!deserializer.atEnd())
        return {};
    else
        return create();
}

void SecondaryNodeReady::serializePayload(std::vector<boost::asio::const_buffer> &) const
{
    // noop
}

const OpCode &SecondaryNodeReady::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::PING_PONG };
    return kOpCode;
}

}    // namespace Proto2::Response
