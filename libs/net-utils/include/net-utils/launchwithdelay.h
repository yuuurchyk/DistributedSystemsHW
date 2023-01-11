#pragma once

#include <functional>

#include <boost/asio.hpp>

namespace NetUtils
{
void launchWithDelay(boost::asio::io_context &, boost::posix_time::milliseconds delay, std::function<void()> callback);

}    // namespace NetUtils
