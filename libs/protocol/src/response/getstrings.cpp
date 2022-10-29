#include "protocol/response/getstrings.h"

#include <stdexcept>
#include <utility>

#include <boost/scope_exit.hpp>

namespace protocol::response
{
GetStrings GetStrings::answer(const request::GetStrings           &request,
                              const std::vector<std::string_view> &body)
{
    auto bodySize = size_t{};

    bodySize += body.size() * sizeof(size_t);
    for (const auto &str : body)
        bodySize += str.size();

    auto builder = Response::responseFrameBuilder(request, bodySize);

    for (const auto &str : body)
    {
        builder.addToBody(str.size());
        builder.addToBody(str);
    }

    auto response = GetStrings{ Response{ builder.build() } };

    if (!request.valid())
        response.invalidate();

    return response;
}

GetStrings::GetStrings(Response &&response) : Response{ std::move(response) }
{
    decideOnValidity();
}

const std::vector<std::string_view> &GetStrings::strings() const noexcept
{
    return strings_;
}

void GetStrings::decideOnValidity()
{
    if (!valid())
        return;

    if (opCode() != codes::OpCode::GET_STRINGS)
        return invalidate();

    const auto body = this->body();
    auto       l    = body.data();
    const auto r    = body.data() + body.size();

    auto invalidateAndClear = [&]()
    {
        invalidate();
        strings_.clear();
    };

    try
    {
        while (l < r)
        {
            if (r - l < sizeof(size_t))
                return invalidateAndClear();

            const auto size = *reinterpret_cast<const size_t *>(l);
            l += sizeof(size_t);

            if (l + size - 1 >= r)
                return invalidateAndClear();
            else
            {
                strings_.push_back(
                    std::string_view{ reinterpret_cast<const char *>(l), size });
            }
        }
    }
    catch (const std::exception &)
    {
        invalidateAndClear();
    }
}

}    // namespace protocol::response
