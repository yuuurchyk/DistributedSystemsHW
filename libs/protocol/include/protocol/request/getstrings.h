#pragma once

#include "protocol/request/request.h"

namespace protocol::request
{

class GetStrings : public Request
{
public:
    static GetStrings form(size_t requestId);

    GetStrings(Request &&);

private:
    void decideOnValidity();
};

}    // namespace protocol::request
