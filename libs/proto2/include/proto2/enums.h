#pragma once

#include <cstdint>

namespace Proto2
{

// used for add message response
enum class AddMessageStatus : uint8_t
{
    OK = 0,
    NOT_ALLOWED
};

}    // namespace Proto2
