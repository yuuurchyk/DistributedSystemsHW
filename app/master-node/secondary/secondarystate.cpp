#include "secondarystate.h"

std::ostream &operator<<(std::ostream &strm, SecondaryState rhs)
{
    switch (rhs)
    {
    case SecondaryState::INVALIDATED:
        return strm << "SecondaryState::INVALIDATED";
    case SecondaryState::REGISTERING:
        return strm << "SecondaryState::REGISTERING";
    case SecondaryState::OPERATIONAL:
        return strm << "SecondaryState::OPERATIONAL";
    }

    return strm << "SecondaryState(invalid)";
}
