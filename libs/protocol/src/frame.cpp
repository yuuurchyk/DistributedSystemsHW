#include "protocol/frame.h"

#include <utility>

namespace protocol
{
Frame::Frame(Buffer buffer) : buffer_{ std::move(buffer) }
{
    decideOnValidity();
}

void Frame::decideOnValidity()
{
    if (buffer_.invalidated())
        return invalidate();

    if (buffer_.size() < kBodyOffset)
        return invalidate();

    if (*reinterpret_cast<const size_type *>(buffer_.data() + kFrameSizeOffset) !=
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

Frame::size_type Frame::requestId() const
{
    if (!valid())
        return {};
    else
        return *reinterpret_cast<const size_type *>(buffer_.data() + kRequestIdOffset);
}

Buffer Frame::flushBuffer()
{
    return std::move(buffer_);
}

BufferView Frame::body()
{
    if (!valid())
        return BufferView{ nullptr, 0 };
    else
        return BufferView{ buffer_.data() + kBodyOffset, buffer_.size() - kBodyOffset };
}

void Frame::invalidate()
{
    valid_ = false;
}

}    // namespace protocol
