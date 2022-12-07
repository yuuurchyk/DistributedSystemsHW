#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "proto/detail/traits.h"
#include "proto/timestamp.h"

/**
 * @brief Types that can be serializabled/deserialized:
 * 1. integral types
 * 2. enums
 * 3. string
 * 4. any combinations of optional, vector
 * 5. custom classes, if they define 2 tie() overloads (one should return
 *    tuple of referenes, other should return tuple of const references).
 */
namespace Proto
{
using RequestId_t = uint64_t;

enum class EventType : uint8_t
{
    REQUEST = 0,
    RESPONSE
};
enum class OpCode : uint8_t
{
    ADD_MESSAGE = 0,
    GET_MESSAGES
};
enum class ResponseStatus : uint8_t
{
    RECIEVED_FINE = 0,
    RECIEVED_BAD_FRAME,
    NOT_RECIEVED_TIMEOUT,
    NOT_RECEIVED_DISCONNECTED
};

struct Message
{
    Timestamp_t timestamp;
    std::string message;

    auto tie() const { return std::tie(timestamp, message); }
    auto tie() { return std::tie(timestamp, message); }
};

namespace Request
{
    struct AddMessage
    {
        Message message;

        auto tie() const { return std::tie(message); }
        auto tie() { return std::tie(message); }
    };

    struct GetMessages
    {
        auto tie() const { return std::make_tuple(); }
        auto tie() { return std::make_tuple(); }
    };
}    // namespace Request

namespace Response
{
    struct AddMessage
    {
        enum class Status : uint8_t
        {
            OK = 0,
            NOT_ALLOWED
        };

        Status status{ Status::NOT_ALLOWED };

        auto tie() { return std::tie(status); }
        auto tie() const { return std::tie(status); }
    };

    struct GetMessages
    {
        std::vector<Message> messages;

        auto tie() { return std::tie(messages); }
        auto tie() const { return std::tie(messages); }
    };
}    // namespace Response

PROTO_REG_REQUEST_RESPONSE(Request::AddMessage, Response::AddMessage, OpCode::ADD_MESSAGE)
PROTO_REG_REQUEST_RESPONSE(Request::GetMessages,
                           Response::GetMessages,
                           OpCode::GET_MESSAGES)

}    // namespace Proto
