#include "proto/serialization.h"

#include <algorithm>

namespace Proto
{
SerializationContext::SerializationContext(size_t bytesNum)
{
    bytesNum = std::max(bytesNum, kDefaultChunkSize);
    allocateChunk(bytesNum);
}

std::byte &SerializationContext::getContiguousMemory(size_t bytesNum)
{
    if (currentChunk_ + bytesNum < currentChunkEnd_)
        allocateChunk(std::max(bytesNum, kDefaultChunkSize));

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

}    // namespace Proto

namespace Proto::detail::Serialization
{

size_t additionalBytesRequired(const std::string &s)
{
    return sizeof(std::string::size_type);
}

size_t buffersRequired(const std::string &s)
{
    return 2;
}

void serialize(const std::string                      &s,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    serialize(context.add(s.size()), seq, context);
    seq.emplace_back(s.data(), s.size());
}

}    // namespace Proto::detail::Serialization
