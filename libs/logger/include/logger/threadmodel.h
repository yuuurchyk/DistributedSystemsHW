#pragma once

#include <boost/log/detail/light_rw_mutex.hpp>
#include <boost/log/sources/threading_models.hpp>

namespace logger
{

using single_thread_model = boost::log::sources::single_thread_model;
using multi_thread_model =
    boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>;

}    // namespace logger
