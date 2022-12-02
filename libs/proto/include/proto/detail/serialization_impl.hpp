#pragma once

#include "proto/serialization.h"

#include <optional>
#include <string>
#include <utility>

namespace Proto::detail
{
template <typename T>
void serialize(const T &, SerializationContext &);
template <typename T>
void serialize(const std::optional<T> &, SerializationContext &);
template <typename T>
void serialize(const std::vector<T> &, SerializationContext &);
template <typename... Args>
void serialize(const std::tuple<Args...> &, SerializationContext &);
template <>
void serialize<std::string>(const std::string &, SerializationContext &);

template <typename T>
void serialize(const T &val, SerializationContext &context)
{
    static_assert(std::is_same_v<T, std::decay_t<T>>);

    if constexpr (std::is_pod_v<T>)
    {
        context.constBufferSequence.emplace_back(&val, sizeof(T));
    }
    else
    {
        serialize(val.tie(), context);
    }
}

template <typename T>
void serialize(const std::optional<T> &opt, SerializationContext &context)
{
    serialize(context.add(opt.has_value()), context);

    if (opt.has_value())
        serialize(opt.value(), context);
}

template <typename T>
void serialize(const std::vector<T> &v, SerializationContext &context)
{
    serialize(context.add(v.size()), context);

    for (const auto &entry : v)
        serialize(entry, context);
}

template <typename... Args>
void serialize(const std::tuple<Args...> &t, SerializationContext &context)
{
    std::apply([&](const auto &...val) { (..., serialize(val, context)); }, t);
}

}    // namespace Proto::detail

namespace Proto
{
template <typename T, typename>
const T &SerializationContext::add(T value)
{
    static_assert(std::is_same_v<std::decay_t<T>, T>);
    static_assert(std::is_pod_v<T>);

    auto ptr = new T{ value };
    dtors_.push_back([ptr]() { delete ptr; });

    return *ptr;
}

template <typename Event>
std::unique_ptr<SerializationContext> serialize(Event e)
{
    static_assert(std::is_same_v<Event, std::decay_t<Event>>);
    static_assert(std::is_default_constructible_v<Event>);
    static_assert(std::is_move_constructible_v<Event>);

    auto context = std::make_unique<SerializationContext>();

    auto eventOwner = std::make_unique<Event>(std::move(e));
    auto event      = eventOwner.get();
    context->dtors_.push_back([event = eventOwner.get()]() { delete event; });
    eventOwner.release();

    detail::serialize(Event::kEventType, *context);
    detail::serialize(Event::kOpCode, *context);
    detail::serialize(event->tie(), *context);

    return context;
}

}    // namespace Proto
