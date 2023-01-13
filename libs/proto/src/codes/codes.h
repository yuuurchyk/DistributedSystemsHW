#pragma once

#include <cstdint>
#include <ostream>

namespace Proto
{
enum class EventType : uint8_t
{
    REQUEST = 0,
    RESPONSE
};
enum class OpCode : uint8_t
{
    ADD_MESSAGE = 0,
    GET_MESSAGES,
    SECONDARY_NODE_READY,
    PING_PONG
};

std::ostream &operator<<(std::ostream &, EventType);
std::ostream &operator<<(std::ostream &, OpCode);

}    // namespace Proto
