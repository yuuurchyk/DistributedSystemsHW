#pragma once

#include <chrono>
#include <cstddef>
#include <utility>

namespace Constants2
{
using duration_milliseconds_t = std::chrono::duration<size_t, std::milli>;

static constexpr duration_milliseconds_t                                     kOutcomingRequestTimeout{ 1500 };
static constexpr std::pair<duration_milliseconds_t, duration_milliseconds_t> kArtificialSendDelayBounds{
    duration_milliseconds_t{ 1000 },
    duration_milliseconds_t{ 2000 }
};

}    // namespace Constants2
