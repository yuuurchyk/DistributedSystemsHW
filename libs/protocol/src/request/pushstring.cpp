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
        return;

    if (opCode() != codes::OpCode::PUSH_STRING)
        return invalidate();

    const auto body = this->body();

    if (body.size() == 0)
        content_ = "";
    else
        content_ =
            std::string_view{ reinterpret_cast<const char *>(body.data()), body.size() };
}

}    // namespace protocol::request
