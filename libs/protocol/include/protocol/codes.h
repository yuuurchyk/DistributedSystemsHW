#pragma once

#include <cstdint>

/**
 * @brief Structure of Frame:
 * 8 bytes: frame size
 * 1 byte:  EventType
 * 1 byte:  RequestType | ResponseType
 * 8 bytes: id (for request: request id, for response: id of the incoming request)
 * body     request/response specific payload
 */
namespace protocol::codes
{
enum class EventType : uint8_t
{
    REQUEST = 0,
    RESPONSE,
    ERROR
};

enum class RequestType : uint8_t
{
    REQUEST_PUSH_STRING = 0,
    REQUEST_GET_STRINGS,
    ERROR
};

enum class ResponseType : uint8_t
{
    REPONSE_PUSH_STRING = 0,
    RESPONSE_GET_STRINGS,
    ERROR
};

}    // namespace protocol::codes
