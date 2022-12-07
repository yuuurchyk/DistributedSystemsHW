#include "reflection/serialization.h"

namespace Reflection
{
const std::vector<boost::asio::const_buffer> &
    SerializationContext::constBufferSequence() const noexcept
{
    return seq_;
}

std::byte &SerializationContext::getContiguousMemory(size_t bytesNum)
{
    if (currentChunk_ + bytesNum < currentChunkEnd_)
        allocateChunk(bytesNum);

    auto &res = *currentChunk_;
    currentChunk_ += bytesNum;
    return res;
}

void SerializationContext::allocateChunk(size_t bytesNum)
{
    chunks_.emplace_back(new std::byte[bytesNum]);
    currentChunk_    = chunks_.back().get();
    currentChunkEnd_ = currentChunk_ + bytesNum;
}

size_t SerializationContext::buffersRequired(const std::string &)
{
    return 2;
}

size_t SerializationContext::additionalBytesRequired(const std::string &)
{
    return sizeof(std::string::size_type);
}

void SerializationContext::serializeImpl(const std::string &s)
{
    serializeImpl(add(s.size()));
    seq_.emplace_back(s.data(), s.size());
}

}    // namespace Reflection
