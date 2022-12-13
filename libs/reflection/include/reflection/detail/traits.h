#pragma once

#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Reflection::detail::Traits
{
template <typename T>
struct Serializable;
template <typename T>
struct Deserializable;

template <typename T>
struct TriviallySerializable
    : std::bool_constant<std::is_same_v<T, std::decay_t<T>> &&
                         (std::is_integral_v<T> || std::is_enum_v<T>)>
{
};
template <typename T>
struct TriviallyDeserializable : std::bool_constant<TriviallySerializable<T>::value>
{
};

template <typename T>
struct SerializableCTieEntry
    : std::bool_constant<
          std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>> &&
          Serializable<std::remove_const_t<std::remove_reference_t<T>>>::value>
{
};
template <typename T>
struct SerializableCTie : std::false_type
{
};
template <typename... Args>
struct SerializableCTie<std::tuple<Args...>>
    : std::bool_constant<std::conjunction<SerializableCTieEntry<Args>...>::value>
{
};

template <typename T>
struct DeserializableTieEntry
    : std::bool_constant<std::is_reference_v<T> &&
                         Deserializable<std::remove_reference_t<T>>::value>
{
};
template <typename T>
struct DeserializableTie : std::false_type
{
};
template <typename... Args>
struct DeserializableTie<std::tuple<Args...>>
    : std::bool_constant<std::conjunction<DeserializableTieEntry<Args>...>::value>
{
};

template <typename T>
inline constexpr bool HasSerializableCTie_v = requires(const T &val)
{
    requires std::is_same_v<T, std::decay_t<T>>;

    val.tie();
    requires std::is_same_v<decltype(val.tie()), std::decay_t<decltype(val.tie())>>;
    requires SerializableCTie<decltype(val.tie())>::value;
};
template <typename T>
inline constexpr bool HasDeserializableTie_v = requires(T &val)
{
    requires std::is_same_v<T, std::decay_t<T>>;

    val.tie();
    requires std::is_same_v<decltype(val.tie()), std::decay_t<decltype(val.tie())>>;
    requires DeserializableTie<decltype(val.tie())>::value;
};

template <typename T>
struct Serializable : public std::bool_constant<TriviallySerializable<T>::value ||
                                                HasSerializableCTie_v<T>>
{
};
template <typename T>
struct Deserializable : public std::bool_constant<TriviallyDeserializable<T>::value ||
                                                  HasDeserializableTie_v<T>>
{
};

template <typename T>
struct Serializable<std::vector<T>> : std::bool_constant<Serializable<T>::value>
{
};
template <typename T>
struct Deserializable<std::vector<T>> : std::bool_constant<Deserializable<T>::value>
{
};

template <>
struct Serializable<std::string>
    : std::bool_constant<Serializable<std::string::value_type>::value>
{
};
template <>
struct Deserializable<std::string>
    : std::bool_constant<Deserializable<std::string::value_type>::value>
{
};

// TODO: tuples, pairs, arrays, floating point

}    // namespace Reflection::detail::Traits
