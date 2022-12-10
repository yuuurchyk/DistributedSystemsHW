#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <tuple>
#include <vector>

#include "proto/timestamp.h"

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
    GET_MESSAGES
};
}    // namespace Proto

#include "proto/concepts.h"
#include "proto/detail/traits.h"

namespace Proto
{
// exception structs used with response futures
struct ResponseException : public std::exception
{
};
struct BadFrameException : public ResponseException
{
    const char *what() const noexcept { return "Bad response frame format"; }
};
struct TimeoutException : public ResponseException
{
    const char *what() const noexcept { return "Timeout occured for response"; }
};
struct DisconnectedException : public ResponseException
{
    const char *what() const noexcept { return "Peer disconnected"; }
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
        Timestamp_t timestamp;
        std::string message;

        auto tie() const { return std::tie(timestamp, message); }
        auto tie() { return std::tie(timestamp, message); }
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
PROTO_REG_REQUEST_RESPONSE(Request::GetMessages, Response::GetMessages, OpCode::GET_MESSAGES)

}    // namespace Proto
