#pragma once

#include <type_traits>

#include "proto/detail/traits.h"
#include "proto/proto.h"

namespace Proto::Concepts
{
template <typename T>
concept HasSerializableTie = detail::Traits::HasSerializableCTie_v<T>;
template <typename T>
concept HasDeserializableTie = detail::Traits::HasDeserializableTie_v<T>;

template <typename T>
concept Serializable = detail::Traits::Serializable<T>::value;
template <typename T>
concept TriviallySerializable =
    Serializable<T> && detail::Traits::TriviallySerializable<T>::value;
template <typename T>
concept NonTriviallySerializable =
    Serializable<T> && !detail::Traits::TriviallySerializable<T>::value;

template <typename T>
concept Deserializable = detail::Traits::Deserializable<T>::value &&
    std::is_default_constructible<T>::value && std::is_move_constructible<T>::value;
template <typename T>
concept TriviallyDeserializable =
    Deserializable<T> && detail::Traits::TriviallyDeserializable<T>::value;
template <typename T>
concept NonTriviallyDeserializable =
    Deserializable<T> && !detail::Traits::TriviallyDeserializable<T>::value;

template <typename T>
concept Event = Serializable<T> && Deserializable<T> && requires(T)
{
    requires detail::Traits::IsEvent<T>::value;
    detail::Traits::EventType<T>::value;
    detail::Traits::OpCode<T>::value;
};
template <typename T>
concept Request = Event<T> && requires(T)
{
    requires detail::Traits::EventType<T>::value == Proto::EventType::REQUEST;
    requires detail::Traits::EventType<
        typename detail::Traits::Response<T>::type>::value == Proto::EventType::RESPONSE;
};
template <typename T>
concept Response = Event<T> and requires(T)
{
    requires detail::Traits::EventType<T>::value == EventType::RESPONSE;
    requires detail::Traits::EventType<
        typename detail::Traits::Request<T>::type>::value == Proto::EventType::REQUEST;
};

template <Event Event>
inline constexpr OpCode OpCode_v = detail::Traits::OpCode<Event>::value;
template <Request Request>
using Response_t = typename detail::Traits::Response<Request>::type;
template <Response Response>
using Request_t = typename detail::Traits::Request<Response>::type;

}    // namespace Proto::Concepts
