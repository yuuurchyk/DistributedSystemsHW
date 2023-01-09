#include "proto2/duration.h"

namespace Proto2
{

boost::posix_time::milliseconds toPosixTime(duration_milliseconds_t duration)
{
    return boost::posix_time::milliseconds{ duration.count() };
}

}    // namespace Proto2
