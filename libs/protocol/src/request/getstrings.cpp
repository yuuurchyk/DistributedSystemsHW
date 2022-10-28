#include "protocol/request/getstrings.h"

#include <utility>

namespace protocol::request
{

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
