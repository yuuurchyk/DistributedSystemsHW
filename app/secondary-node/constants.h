#pragma once

#include <cstddef>

namespace Constants
{
// secondary node will schedule reconnect to master request
// at regular time intervals
constexpr size_t kMasterReconnectTimeoutMs{ 3000 };

// if no response from master within this time frame is recieved,
// request is considered timed out
constexpr size_t kMasterSendTimeoutMs{ 3000 };

}    // namespace Constants
