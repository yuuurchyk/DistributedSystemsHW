#include "proto2/timestamp.h"

#include <chrono>

namespace Proto2
{
Timestamp_t getCurrentTimestamp()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    Timestamp_t timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    return timestamp;
}

}    // namespace Proto2
