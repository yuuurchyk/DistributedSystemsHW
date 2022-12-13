#pragma once

#include <cstdint>

namespace Proto
{

using Timestamp_t = uint64_t;
/**
 * @brief returns current timestamp
 * @note returned timestamp is guaranteed to be unique even
 * during concurrent execution (mutex is used inside)
 */
Timestamp_t getCurrentTimestamp();

}    // namespace Proto
