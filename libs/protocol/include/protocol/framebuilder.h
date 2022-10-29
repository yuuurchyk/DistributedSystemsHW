#pragma once

#include <cstddef>

#include "utils/copymove.h"

#include "protocol/buffer.h"
#include "protocol/codes.h"
#include "protocol/frame.h"

namespace protocol
{
class FrameBuilder
{
    DISABLE_COPY(FrameBuilder);

public:
    FrameBuilder(codes::Event, codes::OpCode, size_t requestId);
    /**
     * @brief allows for faster construction when we know body size
     * beforehand
     */
    FrameBuilder(codes::Event, codes::OpCode, size_t id, size_t bodySize);

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
