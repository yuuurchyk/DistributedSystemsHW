#pragma once

#include <cstddef>
#include <stdexcept>
#include <string_view>

#include <boost/predef/os/unix.h>

namespace logger::detail
{
#ifdef BOOST_OS_UNIX
constexpr std::string_view getBaseNameImpl(const std::string_view &s, size_t i)
{
    if (s.at(s.size() - 1 - i) == '/')
        return std::string_view{ s.data() + s.size() - i, i };
    else
        return getBaseNameImpl(s, i + 1);
}

constexpr std::string_view getBaseName(const std::string_view &s)
{
    return getBaseNameImpl(s, 0);
}
#else
#    error "Unsupported OS"
#endif

}    // namespace logger::detail

#define __FILENAME__ ::logger::detail::getBaseName(std::string_view{ __FILE__ })
