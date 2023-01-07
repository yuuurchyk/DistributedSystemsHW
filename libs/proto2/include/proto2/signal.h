#pragma once

#include <boost/signals2.hpp>

namespace Proto2
{
template <typename... Args>
using signal = boost::signals2::signal<void(Args...)>;

}    // namespace Proto2
