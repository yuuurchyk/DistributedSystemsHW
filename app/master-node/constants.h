#pragma once

#include <cstddef>

namespace Constants
{
// timer is activated when write concern is not satisfied, but we still need to
// wait for confirmations from secondaries
constexpr size_t kSecondariesPollingDelayMs{ 3000 };

// if no response from secondary within this time frame is recieved,
// request is considered timed out
constexpr size_t kSecondarySendTimeoutMs{ 3000 };

}    // namespace Constants

static_assert(Constants::kSecondariesPollingDelayMs > 0);
static_assert(Constants::kSecondarySendTimeoutMs);
