#pragma once

#include <cstdint>
#include <ostream>

namespace Proto2
{
// used for add message response
enum class AddMessageStatus : uint8_t
{
    OK = 0,
    NOT_ALLOWED
};

std::ostream &operator<<(std::ostream &, AddMessageStatus);

}    // namespace Proto2
