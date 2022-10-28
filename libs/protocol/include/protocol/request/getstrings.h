#pragma once

#include "protocol/request/request.h"

namespace protocol::request
{

class GetStrings : public Request
{
public:
    GetStrings(Request &&);

private:
    void decideOnValidity();
};

}    // namespace protocol::request
