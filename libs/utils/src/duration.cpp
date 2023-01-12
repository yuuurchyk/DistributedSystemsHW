#include "utils/duration.h"

namespace Utils
{
boost::posix_time::milliseconds toPosixTime(duration_milliseconds_t duration)
{
    return boost::posix_time::milliseconds{ duration.count() };
}

}    // namespace Utils
