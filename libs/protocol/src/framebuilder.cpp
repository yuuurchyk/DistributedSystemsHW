#include "protocol/framebuilder.h"

namespace protocol
{

FrameBuilder::FrameBuilder(codes::Event event, codes::OpCode opcode, size_t id)
    : FrameBuilder{ event, opcode, id, /* bodySize */ 0 }
{
}

FrameBuilder::FrameBuilder(codes::Event  event,
                           codes::OpCode opcode,
                           size_t        id,
                           size_t        bodySize)
    : buffer_{ sizeof(size_t) + sizeof(codes::Event) + sizeof(codes::OpCode) +
               sizeof(size_t) + bodySize }
{
    buffer_ << size_t{} << event << opcode << id;
}

Frame FrameBuilder::build()
{
    if (!buffer_.invalidated())
        *reinterpret_cast<size_t *>(buffer_.data()) = buffer_.size();
    return Frame{ std::move(buffer_) };
}

}    // namespace protocol
