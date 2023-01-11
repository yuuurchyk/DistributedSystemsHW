#pragma once

#include <cstdint>

namespace Proto2
{
using Timestamp_t = uint64_t;
/**
 * @brief returns current timestamp
 */
Timestamp_t getCurrentTimestamp();

}    // namespace Proto2
