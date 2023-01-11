#include "logger/severity.h"

#include <array>

namespace logger
{
std::ostream &operator<<(std::ostream &strm, Severity severity)
{
    static constexpr std::array<const char *, 4> kRepr{ "DBG ", "Info", "Warn", "ERR!" };

    const auto i = static_cast<unsigned int>(severity);
    if (i < kRepr.size())
        return strm << kRepr[i];
    else
        return strm << i;
}

}    // namespace logger
