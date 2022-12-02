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
 * 3. tie() methods should return tuple of references or const references
 * 4. static kEventType entry
 * 5. static kOpCode entry
 * 6. relatively simple members (pods, vectors, optionals, strings are supported)
 *    (also custom classes with tie() methods are supported for
 * serialization/deserialization)
 *
 * @note 2 overloads of tie() should be present (const references for serialization and
 * references for deserialization)
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

        static constexpr EventType kEventType{ EventType::REQUEST };
        static constexpr OpCode    kOpCode{ OpCode::ADD_MESSAGE };
        auto                       tie() const { return std::tie(message); }
        auto                       tie() { return std::tie(message); }
    };

    struct GetMessages
    {
        static constexpr EventType kEventType{ EventType::REQUEST };
        static constexpr OpCode    kOpCode{ OpCode::GET_MESSAGES };
        auto                       tie() const { return std::make_tuple(); }
        auto                       tie() { return std::make_tuple(); }
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

        static constexpr EventType kEventType{ EventType::RESPONSE };
        static constexpr OpCode    kOpCode{ OpCode::ADD_MESSAGE };
        auto                       tie() { return std::tie(status); }
        auto                       tie() const { return std::tie(status); }
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

        static constexpr EventType kEventType{ EventType::RESPONSE };
        static constexpr OpCode    kOpCode{ OpCode::GET_MESSAGES };
        auto                       tie() { return std::tie(status, messages); }
        auto                       tie() const { return std::tie(status, messages); }
    };
}    // namespace Response

}    // namespace Proto
