#include "protocol/request/pushstring.h"

#include <utility>

#include "protocol/framebuilder.h"

namespace protocol::request
{
PushString PushString::form(size_t requestId, const std::string &s)
{
    return form(requestId, s.data(), s.size());
}

PushString PushString::form(size_t requestId, std::string_view s)
{
    return form(requestId, s.data(), s.size());
}

PushString PushString::form(size_t requestId, const char *s, size_t size)
{
    return PushString{ Request{
        FrameBuilder{ codes::Event::REQUEST, codes::OpCode::PUSH_STRING, requestId, size }
            .addToBody(std::string_view{ s, size })
            .build() } };
}

PushString::PushString(Request &&request) : Request{ std::move(request) }
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
