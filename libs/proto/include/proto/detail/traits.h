#pragma once

#include <type_traits>

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
