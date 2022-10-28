#include "protocol/codes.h"

#include <algorithm>
#include <array>

namespace protocol::codes
{
std::ostream &operator<<(std::ostream &strm, Event rhs)
{
    static constexpr std::array<const char *, 3> kNames{ "REQUEST", "RESPONSE", "ERROR" };

    const auto index = std::min(kNames.size() - 1, static_cast<size_t>(rhs));
    return strm << "Event::" << kNames[index];
}

std::ostream &operator<<(std::ostream &strm, OpCode rhs)
{
    static constexpr std::array<const char *, 3> kNames{ "PUSH_STRING",
                                                         "GET_STRINGS",
                                                         "ERROR" };

    const auto index = std::min(kNames.size() - 1, static_cast<size_t>(rhs));
    return strm << "OpCode::" << kNames[index];
}

std::ostream &operator<<(std::ostream &strm, Response rhs)
{
    static constexpr std::array<const char *, 3> kNames{ "SUCCESS", "FAILURE", "ERROR" };

    const auto index = std::min(kNames.size() - 1, static_cast<size_t>(rhs));
    return strm << "Response::" << kNames[index];
}

}    // namespace protocol::codes
