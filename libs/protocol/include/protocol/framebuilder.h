#pragma once

#include <cstddef>

#include "protocol/buffer.h"
#include "protocol/codes.h"
#include "protocol/frame.h"

namespace protocol
{
class FrameBuilder
{
public:
    FrameBuilder(codes::Event, codes::OpCode, Frame::size_type requestId);
    /**
     * @brief allows for faster construction when we know body size
     * beforehand
     */
    FrameBuilder(codes::Event, codes::OpCode, Frame::size_type id, size_t bodySize);

    FrameBuilder(const FrameBuilder &)            = delete;
    FrameBuilder(FrameBuilder &&)                 = delete;
    FrameBuilder &operator=(const FrameBuilder &) = delete;
    FrameBuilder &operator=(FrameBuilder &&)      = delete;
    ~FrameBuilder()                               = default;

    template <typename T>
    FrameBuilder &addToBody(const T &val)
    {
        buffer_ << val;
        return *this;
    }

    Frame build();

private:
    Buffer buffer_;
};

}    // namespace protocol
