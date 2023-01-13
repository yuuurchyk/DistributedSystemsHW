#include "codes/codes.h"

namespace Proto
{
std::ostream &operator<<(std::ostream &strm, EventType rhs)
{
    switch (rhs)
    {
    case EventType::REQUEST:
        return strm << "EventType::REQUEST";
    case EventType::RESPONSE:
        return strm << "EventType::RESPONSE";
    }

    return strm << "EventType(invalid)";
}

std::ostream &operator<<(std::ostream &strm, OpCode rhs)
{
    switch (rhs)
    {
    case OpCode::ADD_MESSAGE:
        return strm << "OpCode::ADD_MESSAGE";
    case OpCode::GET_MESSAGES:
        return strm << "OpCode::GET_MESSAGES";
    case OpCode::SECONDARY_NODE_READY:
        return strm << "OpCode::SECONDARY_NODE_READY";
    case OpCode::PING_PONG:
        return strm << "OpCode::PING_PONG";
    }

    return strm << "OpCode(invalid)";
}

}    // namespace Proto
