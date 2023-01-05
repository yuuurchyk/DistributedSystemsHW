#include "buffersequenceserializer.h"

#include <cassert>

namespace Proto2
{
BufferSequenceSerializer::BufferSequenceSerializer(std::vector<boost::asio::const_buffer> &seq) : seq_{ seq } {}

void BufferSequenceSerializer::serialize(const std::byte *memory, size_t size)
{
    if (memory == nullptr)
        return;

    seq_.push_back(boost::asio::buffer(memory, size));
}

}    // namespace Proto2
