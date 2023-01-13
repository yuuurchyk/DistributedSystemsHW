#include "outcomingrequestcontext/invalidationreason.h"

namespace Proto::OutcomingRequestContext
{
std::ostream &operator<<(std::ostream &strm, InvalidationReason rhs)
{
    switch (rhs)
    {
    case InvalidationReason::TIMEOUT:
        return strm << "InvalidationReason::TIMEOUT";
    case InvalidationReason::BAD_RESPONSE_FRAME:
        return strm << "InvalidationReason::BAD_RESPONSE_FRAME";
    case InvalidationReason::PEER_DISCONNECTED:
        return strm << "InvalidationReason::PEER_DISCONNECTED";
    }

    return strm << "InvalidationReason(invalid)";
}

}    // namespace Proto::OutcomingRequestContext
