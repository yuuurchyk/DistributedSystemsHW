#pragma once

#include <cstdint>

namespace Utils
{
using Timestamp_t = uint64_t;
/**
 * @brief returns current timestamp
 */
Timestamp_t getCurrentTimestamp();

}    // namespace Utils
