#pragma once

#include <cstdint>
#include <ostream>

namespace protocol::codes
{
enum class Event : uint8_t
{
    REQUEST = 0,
    RESPONSE,
    ERROR
};
std::ostream &operator<<(std::ostream &, Event);

enum class OpCode : uint8_t
{
    PUSH_STRING = 0,
    GET_STRINGS,
    ERROR
};
std::ostream &operator<<(std::ostream &, OpCode);

enum class Response : uint8_t
{
    SUCCESS = 0,
    FAILURE,
    ERROR
};
std::ostream &operator<<(std::ostream &, Response);

}    // namespace protocol::codes
