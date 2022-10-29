#pragma once

#include <string>
#include <string_view>

#include "protocol/request/request.h"

namespace protocol::request
{
/**
 * @brief PushString request body:
 * ... string to push
 */
class PushString : public Request
{
public:
    static PushString form(size_t requestId, const std::string &);
    static PushString form(size_t requestId, std::string_view);

    PushString(Request &&);

    const std::string_view &string() const;

private:
    static PushString form(size_t requestId, const char *s, size_t size);

    void decideOnValidity();

    std::string_view content_;
};

}    // namespace protocol::request
