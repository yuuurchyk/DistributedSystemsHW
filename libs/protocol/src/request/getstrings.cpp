#include "protocol/request/getstrings.h"

#include <utility>

#include "protocol/framebuilder.h"

namespace protocol::request
{
GetStrings GetStrings::form(size_t requestId)
{
    return GetStrings{ Request{
        FrameBuilder{ codes::Event::REQUEST, codes::OpCode::GET_STRINGS, requestId }
            .build() } };
}

GetStrings::GetStrings(Request &&request) : Request{ std::move(request) }
{
    decideOnValidity();
}

void GetStrings::decideOnValidity()
{
    if (!valid())
        return;

    if (opCode() != codes::OpCode::GET_STRINGS)
        return invalidate();

    if (body().size() != 0)
        return invalidate();
}

}    // namespace protocol::request
