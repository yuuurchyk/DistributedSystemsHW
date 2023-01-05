#include "bufferdeserializer.h"

namespace Proto2
{

BufferDeserializer::BufferDeserializer(boost::asio::const_buffer buffer)
    : begin_{ reinterpret_cast<const std::byte *>(buffer.data()) },
      end_{ begin_ == nullptr ? nullptr : begin_ + buffer.size() },
      current_{ begin_ }
{
}

bool BufferDeserializer::atEnd() const
{
    return current_ == end_;
}

boost::asio::const_buffer BufferDeserializer::leftover() const
{
    if (begin_ == nullptr)
    {
        return boost::asio::const_buffer{ nullptr, 0 };
    }
    else
    {
        const auto leftoverBytes = end_ - current_;
        return boost::asio::const_buffer{ current_, static_cast<size_t>(leftoverBytes) };
    }
}

const std::byte *BufferDeserializer::getMemoryAndAdvance(size_t bytes)
{
    if (begin_ == nullptr)
        return nullptr;

    const auto leftoverBytes = end_ - current_;

    if (leftoverBytes < bytes)
        return nullptr;

    const auto res = current_;
    current_ += bytes;
    return res;
}

}    // namespace Proto2
