#pragma once

#include <ostream>

namespace Proto::OutcomingRequestContext
{
enum class InvalidationReason
{
    TIMEOUT,
    BAD_RESPONSE_FRAME,
    PEER_DISCONNECTED
};

std::ostream &operator<<(std::ostream &, InvalidationReason);

}    // namespace Proto::OutcomingRequestContext
