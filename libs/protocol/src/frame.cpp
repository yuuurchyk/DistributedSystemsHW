#include "protocol/frame.h"

#include <utility>

namespace protocol
{
Frame::Frame(Buffer buffer) : buffer_{ std::move(buffer) }
{
    decideOnValidity();
}

bool Frame::valid() const noexcept
{
    return valid_;
}

void Frame::decideOnValidity()
{
    if (buffer_.invalidated())
        return invalidate();

    if (buffer_.size() < kBodyOffset)
        return invalidate();

    if (*reinterpret_cast<const size_t *>(buffer_.data() + kFrameSizeOffset) !=
        buffer_.size())
        return invalidate();

    if (event() >= codes::Event::ERROR)
        return invalidate();
    if (opCode() >= codes::OpCode::ERROR)
        return invalidate();
}

codes::Event Frame::event() const
{
    if (!valid())
        return codes::Event::ERROR;
    else
        return *reinterpret_cast<const codes::Event *>(buffer_.data() + kEventOffset);
}

codes::OpCode Frame::opCode() const
{
    if (!valid())
        return codes::OpCode::ERROR;
    else
        return *reinterpret_cast<const codes::OpCode *>(buffer_.data() + kOpCodeOffset);
}

size_t Frame::requestId() const
{
    if (!valid())
        return {};
    else
        return *reinterpret_cast<const size_t *>(buffer_.data() + kRequestIdOffset);
}

Buffer Frame::flushBuffer()
{
    return std::move(buffer_);
}

BufferView Frame::body() const
{
    if (!valid())
        return BufferView{ nullptr, 0 };
    else
        return BufferView{ buffer_.data() + kBodyOffset, buffer_.size() - kBodyOffset };
}

BufferView Frame::buffer() const
{
    if (!valid())
        return BufferView{ nullptr, 0 };
    else
        return BufferView{ buffer_.data(), buffer_.size() };
}

void Frame::invalidate()
{
    valid_ = false;
}

}    // namespace protocol
