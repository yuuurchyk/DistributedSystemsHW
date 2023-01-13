#pragma once

#include <boost/signals2.hpp>

namespace Utils
{
template <typename... Args>
using signal = boost::signals2::signal<void(Args...)>;

template <typename... Args>
using slot_type = typename signal<Args...>::slot_type;

}    // namespace Utils
