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
struct Suitable;

template <typename T>
struct SuitableTieEntry
{
    static constexpr bool getValue()
    {
        if constexpr (!std::is_reference_v<T>)
        {
            return false;
        }
        else
        {
            using Element = std::remove_reference_t<T>;
            return Suitable<Element>::value;
        }
    }
    static constexpr bool value{ getValue() };
};
template <typename T>
struct SuitableCTieEntry
{
    static constexpr bool getValue()
    {
        if constexpr (!std::is_reference_v<T>)
        {
            return false;
        }
        else
        {
            using CElement = std::remove_reference_t<T>;

            if constexpr (!std::is_const_v<CElement>)
            {
                return false;
            }
            else
            {
                using Element = std::remove_const_t<CElement>;
                return Suitable<Element>::value;
            }
        }
    }
    static constexpr bool value{ getValue() };
};
template <typename T>
struct SuitableTie : std::false_type
{
};
template <typename... Args>
struct SuitableTie<std::tuple<Args...>>
    : std::bool_constant<std::conjunction<SuitableTieEntry<Args>...>::value>
{
};
template <typename T>
struct SuitableCTie : std::false_type
{
};
template <typename... Args>
struct SuitableCTie<std::tuple<Args...>>
    : std::bool_constant<std::conjunction<SuitableCTieEntry<Args>...>::value>
{
};

template <typename T>
inline constexpr bool HasTie_t = requires(T &val)
{
    val.tie();
    requires SuitableTie<decltype(val.tie())>::value;
};
template <typename T>
inline constexpr bool hasCTie_t = requires(const T &val)
{
    val.tie();
    requires SuitableCTie<decltype(val.tie())>::value;
};

template <typename T>
struct Suitable
{
    static constexpr bool getValue()
    {
        if constexpr (!std::is_same_v<T, std::decay_t<T>>)
            return false;
        else if constexpr (std::is_integral_v<T>)
            return true;
        else if constexpr (std::is_enum_v<T>)
            return true;
        else
        {
            return HasTie_t<T> && hasCTie_t<T>;
        }
    }
    static constexpr bool value{ getValue() };
};

template <typename T>
struct Suitable<std::vector<T>> : std::bool_constant<Suitable<T>::value>
{
};

template <typename T>
struct Suitable<std::optional<T>> : std::bool_constant<Suitable<T>::value>
{
};

template <>
struct Suitable<std::string> : std::true_type
{
};

template <typename... Args>
struct Suitable<std::tuple<Args...>>
    : std::bool_constant<std::conjunction<Suitable<Args>...>::value>
{
};

template <typename F, typename S>
struct Suitable<std::pair<F, S>>
    : std::bool_constant<Suitable<F>::value && Suitable<S>::value>
{
};

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
