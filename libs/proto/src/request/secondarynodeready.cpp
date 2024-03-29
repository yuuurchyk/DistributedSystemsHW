#include "request/secondarynodeready.h"

#include <utility>

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto::Request
{
std::shared_ptr<SecondaryNodeReady> SecondaryNodeReady::create(std::string secondaryNodeName)
{
    return std::shared_ptr<SecondaryNodeReady>(new SecondaryNodeReady{ std::move(secondaryNodeName) });
}

std::shared_ptr<SecondaryNodeReady> SecondaryNodeReady::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    auto optSecondaryNodeName = deserializer.deserialize<std::string>();

    if (!optSecondaryNodeName.has_value() || !deserializer.atEnd())
        return {};

    return create(std::move(optSecondaryNodeName.value()));
}

void SecondaryNodeReady::serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serializeWoOwnership(secondaryNodeName_);
}

const std::string &SecondaryNodeReady::secondaryNodeName() const noexcept
{
    return secondaryNodeName_;
}

std::string SecondaryNodeReady::flushSecondaryNodeName()
{
    return std::move(secondaryNodeName_);
}

const OpCode &SecondaryNodeReady::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::SECONDARY_NODE_READY };
    return kOpCode;
}

SecondaryNodeReady::SecondaryNodeReady(std::string secondaryNodeName)
    : secondaryNodeName_{ std::move(secondaryNodeName) }
{
}

}    // namespace Proto::Request
