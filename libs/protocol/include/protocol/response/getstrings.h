#pragma once

#include <string_view>
#include <vector>

#include "protocol/request/getstrings.h"
#include "protocol/response/response.h"

namespace protocol::response
{
class GetStrings : public Response
{
public:
    static GetStrings answer(const request::GetStrings &,
                             const std::vector<std::string_view> &body);

    GetStrings(Response &&);

    const std::vector<std::string_view> &strings() const noexcept;

private:
    void decideOnValidity();

    std::vector<std::string_view> strings_;
};

}    // namespace protocol::response
