#include "proto/enums.h"

namespace Proto
{
std::ostream &operator<<(std::ostream &strm, AddMessageStatus rhs)
{
    switch (rhs)
    {
    case AddMessageStatus::OK:
        return strm << "AddMessageStatus::OK";
    case AddMessageStatus::NOT_ALLOWED:
        return strm << "AddMessageStatus::NOT_ALLOWED";
    }

    return strm << "AddMessage(invalid)";
}

}    // namespace Proto
