#pragma once

#include <chrono>
#include <cstddef>
#include <utility>

#include "utils/duration.h"

namespace Constants
{
constexpr Utils::duration_milliseconds_t kOutcomingRequestTimeout{ 1500 };

constexpr std::pair<Utils::duration_milliseconds_t, Utils::duration_milliseconds_t> kArtificialSendDelayBounds{
    Utils::duration_milliseconds_t{ 1000 },
    Utils::duration_milliseconds_t{ 2000 }
};

// secondary specific
constexpr Utils::duration_milliseconds_t kMasterReconnectTimeout{ 3000 };

}    // namespace Constants
