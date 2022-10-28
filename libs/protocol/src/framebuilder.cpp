#include "protocol/framebuilder.h"

namespace protocol
{

FrameBuilder::FrameBuilder(codes::Event event, codes::OpCode opcode, Frame::size_type id)
    : FrameBuilder{ event, opcode, id, /* bodySize */ 0 }
{
}

FrameBuilder::FrameBuilder(codes::Event     event,
                           codes::OpCode    opcode,
                           Frame::size_type id,
                           size_t           bodySize)
    : buffer_{ sizeof(Frame::size_type) + sizeof(codes::Event) + sizeof(codes::OpCode) +
               sizeof(Frame::size_type) + bodySize }
{
    buffer_ << Frame::size_type{} << event << opcode << id;
}

Frame FrameBuilder::build()
{
    if (!buffer_.invalidated())
        *reinterpret_cast<Frame::size_type *>(buffer_.data()) = buffer_.size();
    return Frame{ std::move(buffer_) };
}

}    // namespace protocol
