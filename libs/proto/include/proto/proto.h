#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "proto/timestamp.h"

/**
 * @brief Requirements to request/reponse classes:
 * 1. default ctor
 * 2. move ctor
 * 3. tie() methods should return tuple of std::reference_wrapper (or reference wrapper to
 * const)
 * 4. static kEventType entry (request or response)
 * 5. static kOpCode entry
 *
 * @note Request classes will be serialized (tie() should return cref), Response classes
 * will be serialized and deserialized (2 tie() overloads should be present)
 *
 * @note make sure not to move the objects after tie() is called (since the object changes
 * its address. One way to be safe is to wrap the Request/Response class into a smart
 * pointer)
 *
 * @note smart pointers are not supported for serialization/deserialization
 */
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

struct Message
{
    Timestamp_t timestamp;
    std::string message;

    auto tie() const { return std::tie(timestamp, message); }
    auto tie() { return std::tie(timestamp, message); }
};
// TODO: add hashes

namespace Request
{
    struct AddMessage
    {
        Message message;

    public:
        static constexpr EventType kEventType{ EventType::REQUEST };
        static constexpr OpCode    kOpCode{ OpCode::ADD_MESSAGE };

        auto tie() const { return std::tie(message); }
    };

    struct GetMessages
    {
        static constexpr EventType kEventType{ EventType::REQUEST };
        static constexpr OpCode    kOpCode{ OpCode::GET_MESSAGES };

        auto tie() const { return std::make_tuple(); }
    };
}    // namespace Request

namespace Response
{
    struct AddMessage
    {
        enum class Status : uint8_t
        {
            OK = 0,
            OK_ALREADY_PRESENT,
            FAILED
        };

        Status status{ Status::FAILED };

    public:
        static constexpr EventType kEventType{ EventType::RESPONSE };
        static constexpr OpCode    kOpCode{ OpCode::ADD_MESSAGE };

        auto tie() { return std::tie(status); }
        auto tie() const { return std::tie(status); }
    };

    struct GetMessages
    {
        enum class Status : uint8_t
        {
            OK = 0,
            FAILED
        };

        Status                              status{ Status::FAILED };
        std::optional<std::vector<Message>> messages;

    public:
        static constexpr EventType kEventType{ EventType::RESPONSE };
        static constexpr OpCode    kOpCode{ OpCode::GET_MESSAGES };

        auto tie() { return std::tie(status, messages); }
        auto tie() const { return std::tie(status, messages); }
    };
}    // namespace Response

}    // namespace Proto
