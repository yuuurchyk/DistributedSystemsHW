#pragma once

#include <type_traits>

#include "proto/detail/traits.h"
#include "proto/proto.h"
#include "reflection/concepts.h"

namespace Proto::Concepts
{
template <typename T>
concept Event =
    Reflection::Serializable<T> && Reflection::Deserializable<T> && requires(T) {
                                                                        requires detail::Traits::IsEvent<T>::value;
                                                                        detail::Traits::EventType<T>::value;
                                                                        detail::Traits::OpCode<T>::value;
                                                                    };
template <typename T>
concept Request =
    Event<T> && requires(T) {
                    requires detail::Traits::EventType<T>::value == Proto::EventType::REQUEST;
                    requires detail::Traits::EventType<typename detail::Traits::Response<T>::type>::value ==
                                 Proto::EventType::RESPONSE;
                };
template <typename T>
concept Response =
    Event<T> and requires(T) {
                     requires detail::Traits::EventType<T>::value == EventType::RESPONSE;
                     requires detail::Traits::EventType<typename detail::Traits::Request<T>::type>::value ==
                                  Proto::EventType::REQUEST;
                 };

template <Event Event>
inline constexpr OpCode OpCode_v = detail::Traits::OpCode<Event>::value;
template <Request Request>
using Response_t = typename detail::Traits::Response<Request>::type;
template <Response Response>
using Request_t = typename detail::Traits::Request<Response>::type;

}    // namespace Proto::Concepts
