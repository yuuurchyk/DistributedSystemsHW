#pragma once

#include <string_view>

#include "protocol/request/request.h"

namespace protocol::request
{

class PushString : public Request
{
public:
    PushString(Request);

    const std::string_view &string() const;

private:
    void decideOnValidity();

    std::string_view content_;
};

}    // namespace protocol::request
