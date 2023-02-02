#pragma once

#include <ostream>

enum class SecondaryState
{
    INVALIDATED,
    REGISTERING,
    OPERATIONAL
};

std::ostream &operator<<(std::ostream &strm, SecondaryState);
