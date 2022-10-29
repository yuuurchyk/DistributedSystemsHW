#pragma once

#include "protocol/frame.h"
#include "protocol/request/pushstring.h"
#include "protocol/response/response.h"

namespace protocol::response
{
class PushString : public Response
{
public:
    static PushString answer(const request::PushString &);

    PushString(Response &&);

private:
    void decideOnValidity();
};

}    // namespace protocol::response
