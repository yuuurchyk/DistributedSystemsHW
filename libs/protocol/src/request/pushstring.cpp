#include "protocol/request/pushstring.h"

#include <utility>

namespace protocol::request
{

PushString::PushString(Request request) : Request{ std::move(request) }
{
    decideOnValidity();
}

const std::string_view &PushString::string() const
{
    return content_;
}

void PushString::decideOnValidity()
{
    if (!valid())
        return invalidate();
    if (opCode() != codes::OpCode::PUSH_STRING)
        return invalidate();

    const auto body = this->body();

    if (body.size() < sizeof(Frame::size_type))
        return invalidate();

    const auto stringSize = *reinterpret_cast<const Frame::size_type *>(body.data());

    const auto stringBegin = body.data() + sizeof(Frame::size_type);
    const auto stringEnd   = body.data() + body.size();

    if (stringEnd - stringBegin != stringSize)
        return invalidate();

    content_ =
        std::string_view{ reinterpret_cast<const char *>(stringBegin), stringSize };
}

}    // namespace protocol::request
