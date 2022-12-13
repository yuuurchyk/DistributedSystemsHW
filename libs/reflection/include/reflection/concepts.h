#pragma once

#include "reflection/detail/traits.h"

namespace Reflection
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

}    // namespace Reflection
