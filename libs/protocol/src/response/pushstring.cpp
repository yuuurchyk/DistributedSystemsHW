#include "protocol/response/pushstring.h"

#include "protocol/response/response.h"

namespace protocol::response
{
PushString PushString::answer(const request::PushString &request)
{
    auto response =
        PushString{ Response{ Response::responseFrameBuilder(request).build() } };
    if (!request.valid())
        response.invalidate();
    return response;
}

PushString::PushString(Response &&response) : Response{ std::move(response) }
{
    decideOnValidity();
}

void PushString::decideOnValidity()
{
    if (!valid())
        return;

    if (opCode() != codes::OpCode::PUSH_STRING)
        return invalidate();
}

}    // namespace protocol::response
