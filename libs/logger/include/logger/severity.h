#pragma once

#include <ostream>

namespace logger
{
enum class Severity
{
    Debug,
    Info,
    Warning,
    Error
};

std::ostream &operator<<(std::ostream &, Severity);

}    // namespace logger
