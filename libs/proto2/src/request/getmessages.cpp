#include "proto2/request/getmessages.h"

#include "serialization/buffersequenceserializer.h"

namespace Proto2::Request
{
std::unique_ptr<GetMessages> GetMessages::create(size_t startMessageId)
{
    return std::unique_ptr<GetMessages>{ new GetMessages{ startMessageId } };
}

std::unique_ptr<GetMessages> GetMessages::fromPayload(boost::asio::const_buffer buffer)
{
    if (buffer.size() != sizeof(size_t) || buffer.data() == nullptr)
        return {};

    auto startMessageId = *reinterpret_cast<const size_t *>(buffer.data());
    return create(startMessageId);
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

void GetMessages::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serialize(&startMessageId_);
}

}    // namespace Proto2::Request
