#pragma once

#include <cstddef>
#include <string_view>

#include <boost/predef/os/unix.h>

namespace logger::detail
{
#ifdef BOOST_OS_UNIX
constexpr std::string_view extractBaseNameImpl(const std::string_view &s, size_t i)
{
    if (s.at(s.size() - 1 - i) == '/')
        return std::string_view{ s.data() + s.size() - i, i };
    else
        return extractBaseNameImpl(s, i + 1);
}

constexpr std::string_view extractBaseName(const std::string_view &s)
{
    return extractBaseNameImpl(s, 0);
}
#else
#    error "Unsupported OS"
#endif

}    // namespace logger::detail
