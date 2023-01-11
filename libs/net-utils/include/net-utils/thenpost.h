#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace NetUtils
{

template <typename Future, typename Functor>
void thenPost(Future &&f, boost::asio::io_context &, Functor &&callback);

}    // namespace NetUtils

#include "detail/thenpost_impl.h"
