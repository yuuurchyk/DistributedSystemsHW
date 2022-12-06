#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// specifies classes suitable for serialization/deserialization
namespace Proto::detail::Traits
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
                                                  (std::is_default_constructible_v<T> &&
                                                   HasDeserializableTie_v<T>)>
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

template <typename T>
struct Serializable<std::optional<T>> : std::bool_constant<Serializable<T>::value>
{
};
template <typename T>
struct Deserializable<std::optional<T>> : std::bool_constant<Deserializable<T>::value>
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

}    // namespace Proto::detail::Traits

namespace Proto::detail::Traits
{
template <typename T>
struct IsEvent : std::false_type
{
};

template <typename Event>
struct EventType
{
};

template <typename Event>
struct OpCode
{
};

template <typename Request>
struct Response
{
};

template <typename Response>
struct Request
{
};

}    // namespace Proto::detail::Traits

// NOTE: should be used inside Proto namespace
#define PROTO_REG_REQUEST_RESPONSE(Request_, Response_, OpCode_)                   \
    namespace detail::Traits                                                       \
    {                                                                              \
        template <>                                                                \
        struct IsEvent<Proto::Request_> : public std::true_type                    \
        {                                                                          \
        };                                                                         \
        template <>                                                                \
        struct IsEvent<Proto::Response_> : public std::true_type                   \
        {                                                                          \
        };                                                                         \
        template <>                                                                \
        struct EventType<Proto::Request_>                                          \
        {                                                                          \
            static constexpr Proto::EventType value{ Proto::EventType::REQUEST };  \
        };                                                                         \
        template <>                                                                \
        struct EventType<Proto::Response_>                                         \
        {                                                                          \
            static constexpr Proto::EventType value{ Proto::EventType::RESPONSE }; \
        };                                                                         \
        template <>                                                                \
        struct Response<Proto::Request_>                                           \
        {                                                                          \
            using type = Proto::Response_;                                         \
        };                                                                         \
        template <>                                                                \
        struct Request<Proto::Response_>                                           \
        {                                                                          \
            using type = Proto::Request_;                                          \
        };                                                                         \
        template <>                                                                \
        struct OpCode<Proto::Request_>                                             \
        {                                                                          \
            static constexpr Proto::OpCode value{ Proto::OpCode_ };                \
        };                                                                         \
        template <>                                                                \
        struct OpCode<Proto::Response_>                                            \
        {                                                                          \
            static constexpr Proto::OpCode value{ Proto::OpCode_ };                \
        };                                                                         \
    }
