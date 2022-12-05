#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "proto/detail/proto_impl.h"
#include "proto/timestamp.h"

/**
 * @brief Types that can be serializabled/deserialized:
 * 1. integral types
 * 2. enums
 * 3. string
 * 4. any combinations of pair, tuple, optional, vector
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
    };
}    // namespace Request

namespace Response
{
    struct AddMessage
    {
    };

    struct GetMessages
    {
        std::vector<Message> messages;

        auto tie() { return std::tie(messages); }
        auto tie() const { return std::tie(messages); }
    };
}    // namespace Response

PROTO_REG_REQUEST_RESPONSE(Request::AddMessage, Response::AddMessage, OpCode::ADD_MESSAGE)
// PROTO_REG_REQUEST_RESPONSE(Request::GetMessages,
//                            Response::GetMessages,
//                            OpCode::GET_MESSAGES)

namespace Concepts
{
    template <typename T>
    concept Event = requires(T)
    {
        requires detail::Traits::IsEvent<T>::value;
        requires detail::Traits::EventType<T>::value;
        requires detail::Traits::OpCode<T>::value;
    };
    template <typename T>
    concept Request = Event<T> and requires(T)
    {
        requires detail::Traits::EventType<T>::value == EventType::REQUEST;
        detail::Traits::Response<T>::type;
    };
    template <typename T>
    concept Response = Event<T> and requires(T)
    {
        requires detail::Traits::EventType<T>::value == EventType::RESPONSE;
        detail::Traits::Request<T>::type;
    };
    template <typename T>
    concept Serializable = requires(T)
    {
        requires detail::Traits::Suitable<T>::value;
    };
    template <typename T>
    concept Deserializable = requires(T)
    {
        requires std::is_default_constructible_v<T>;
        requires detail::Traits::Suitable<T>::value;
    };
}    // namespace Concepts

template <Concepts::Event Event>
inline constexpr OpCode OpCode_v = detail::Traits::OpCode<Event>::value;
template <Concepts::Request Request>
using Response_t = typename detail::Traits::Response<Request>::type;
template <Concepts::Response Response>
using Request_t = typename detail::Traits::Request<Response>::type;

}    // namespace Proto
