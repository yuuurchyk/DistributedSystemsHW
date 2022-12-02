#include "proto/timestamp.h"

#include <chrono>
#include <mutex>
#include <thread>

namespace Proto
{
Timestamp_t getCurrentTimestamp()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    static std::mutex  mutex;
    static Timestamp_t previousTimestamp{};

    const auto guard = std::lock_guard{ mutex };

    Timestamp_t timestamp =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    while (timestamp == previousTimestamp)
    {
        std::this_thread::sleep_for(1ms);
        timestamp =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    previousTimestamp = timestamp;
    return timestamp;
}

}    // namespace Proto
