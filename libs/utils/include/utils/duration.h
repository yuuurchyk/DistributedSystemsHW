#pragma once

#include <chrono>
#include <cstddef>

#include <boost/asio.hpp>

namespace Utils
{
using duration_milliseconds_t = std::chrono::duration<size_t, std::milli>;

boost::posix_time::milliseconds toPosixTime(duration_milliseconds_t);

}    // namespace Utils
