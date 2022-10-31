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

void Frame::setRequestId(size_t requestId)
{
    if (!valid())
        return;
    else
        *reinterpret_cast<size_t *>(buffer_.data() + kRequestIdOffset) = requestId;
}

BufferView Frame::body() const
{
    if (!valid())
        return BufferView{ nullptr, 0 };
    else
        return BufferView{ buffer_.data() + kBodyOffset, buffer_.size() - kBodyOffset };
}

const Buffer &Frame::buffer() const
{
    return buffer_;
}

void Frame::invalidate()
{
    valid_ = false;
}

std::ostream &operator<<(std::ostream &strm, const Frame &frame)
{
    if (!frame.valid())
        return strm << "Frame(invalid)";
    else
        return strm << "Frame(" << frame.event() << ", " << frame.opCode()
                    << ", requestId=" << frame.requestId()
                    << ", bodyLen=" << frame.body().size() << ")";
}

}    // namespace protocol
