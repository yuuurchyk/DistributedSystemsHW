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

    if (body.size() < sizeof(size_t))
        return invalidate();

    const auto stringSize = *reinterpret_cast<const size_t *>(body.data());

    const auto stringBegin = body.data() + sizeof(size_t);
    const auto stringEnd   = body.data() + body.size();

    if (stringEnd - stringBegin != stringSize)
        return invalidate();

    content_ =
        std::string_view{ reinterpret_cast<const char *>(stringBegin), stringSize };
}

}    // namespace protocol::request
