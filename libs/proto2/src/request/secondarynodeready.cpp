#include "proto2/request/secondarynodeready.h"

#include "serialization/buffersequenceserializer.h"

#include <utility>

namespace Proto2::Request
{
std::unique_ptr<SecondaryNodeReady> SecondaryNodeReady::create(std::string secondaryNodeName)
{
    return std::unique_ptr<SecondaryNodeReady>(new SecondaryNodeReady{ std::move(secondaryNodeName) });
}

std::unique_ptr<SecondaryNodeReady> SecondaryNodeReady::fromPayload(boost::asio::const_buffer buffer)
{
    if (buffer.data() == nullptr)
        return {};

    static_assert(sizeof(char) == 1);
    auto secondaryNodeName = std::string{ reinterpret_cast<const char *>(buffer.data()), buffer.size() };

    return create(std::move(secondaryNodeName));
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

void SecondaryNodeReady::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serialize(&secondaryNodeName_);
}

}    // namespace Proto2::Request
