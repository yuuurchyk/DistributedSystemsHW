#pragma once

#include "proto/serialization.h"

#include <cstring>

#include <optional>
#include <string>
#include <utility>

namespace Proto::detail::Serialization
{
template <typename T>
size_t buffersRequired(const T &);
template <typename T>
size_t buffersRequired(const T &);
template <typename T>
size_t buffersRequired(const std::optional<T> &);
template <typename T>
size_t buffersRequired(const std::vector<T> &);
size_t buffersRequired(const std::string &);
template <typename... Args>
size_t buffersRequired(const std::tuple<Args...> &);
template <typename F, typename S>
size_t buffersRequired(const std::pair<F, S> &);

template <typename T>
size_t buffersRequired(const T &val)
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
        return 1;
    else
        return buffersRequired(val.tie());
}

template <typename T>
size_t buffersRequired(const std::optional<T> &val)
{
    auto res = size_t{ 1 };

    if (val.has_value())
        res += buffersRequired(val.value());

    return res;
}

template <typename T>
size_t buffersRequired(const std::vector<T> &val)
{
    if constexpr (std::is_integral_v<T>)
    {
        return 2;
    }
    else
    {
        auto res = size_t{ 1 };

        for (const auto &entry : val)
            res += buffersRequired(entry);

        return res;
    }
}

template <typename F, typename S>
size_t buffersRequired(const std::pair<F, S> &p)
{
    return buffersRequired(p.first) + buffersRequired(p.second);
}

template <typename... Args>
size_t buffersRequired(const std::tuple<Args...> &t)
{
    auto res = size_t{ 0 };

    auto visit = [&](const auto &val) { res += buffersRequired(val); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, t);

    return res;
}

template <typename T>
size_t additionalBytesRequired(const T &);
template <typename T>
size_t additionalBytesRequired(const std::optional<T> &);
template <typename T>
size_t additionalBytesRequired(const std::vector<T> &);
size_t additionalBytesRequired(const std::string &);
template <typename... Args>
size_t additionalBytesRequired(const std::tuple<Args...> &);
template <typename F, typename S>
size_t additionalBytesRequired(const std::pair<F, S> &);

template <typename T>
size_t additionalBytesRequired(const T &val)
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
        return 0;
    else
        return additionalBytesRequired(val.tie());
}

template <typename T>
size_t additionalBytesRequired(const std::optional<T> &val)
{
    auto ans = sizeof(bool);
    if (val.has_value())
        ans += additionalBytesRequired(val.value());
    return ans;
}

template <typename T>
size_t additionalBytesRequired(const std::vector<T> &v)
{
    auto ans = sizeof(typename std::vector<T>::size_type);

    for (const auto &entry : v)
        ans += additionalBytesRequired(entry);

    return ans;
}

template <typename... Args>
size_t additionalBytesRequired(const std::tuple<Args...> &t)
{
    auto res = size_t{};

    auto visit = [&](const auto &val) { res += additionalBytesRequired(val); };

    std::apply([&](const auto &...entry) { (..., visit(entry)); }, t);

    return res;
}

template <typename F, typename S>
size_t additionalBytesRequired(const std::pair<F, S> &p)
{
    return additionalBytesRequired(p.first) + additionalBytesRequired(p.second);
}

}    // namespace Proto::detail::Serialization

namespace Proto::detail::Serialization
{
template <typename T>
void serialize(const std::optional<T> &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);
template <typename T>
void serialize(const std::vector<T> &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);
void serialize(const std::string &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);
template <typename... Args>
void serialize(const std::tuple<Args...> &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);
template <typename F, typename S>
void serialize(const std::pair<F, S> &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);

template <typename T>
void serialize(const T                                &val,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        seq.emplace_back(static_cast<const void *>(&val), sizeof(val));
    }
    else
    {
        serialize(val.tie(), seq, context);
    }
}

template <typename T>
void serialize(const std::vector<T>                   &v,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    serialize(context.add(v.size()), seq, context);

    if constexpr (std::is_integral_v<T>)
    {
        seq.emplace_back(v.data(), v.size() * sizeof(T));
    }
    else
    {
        for (const auto &entry : v)
            serialize(entry, seq, context);
    }
}

template <typename T>
void serialize(const std::optional<T>                 &opt,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    serialize(context.add(opt.has_value()), seq, context);
    if (opt.has_value())
        serialize(opt.value(), seq, context);
}

template <typename... Args>
void serialize(const std::tuple<Args...>              &t,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    auto visit = [&](const auto &val) { serialize(val, seq, context); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, t);
}

template <typename F, typename S>
void serialize(const std::pair<F, S>                  &p,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    serialize(p.first, seq, context);
    serialize(p.second, seq, context);
}

}    // namespace Proto::detail::Serialization

namespace Proto
{
template <std::integral T>
const T &SerializationContext::add(T value)
{
    auto &mem = getContiguousMemory(sizeof(T));
    std::memcpy(&mem, &value, sizeof(T));
    return reinterpret_cast<const T &>(mem);
}

template <Concepts::Serializable T>
SerializationResult serialize(const T &val)
{
    auto res = SerializationResult{
        {}, SerializationContext{ detail::Serialization::additionalBytesRequired(val) }
    };
    res.constBufferSequence.reserve(detail::Serialization::buffersRequired(val));

    detail::Serialization::serialize(val, res.constBufferSequence, res.context);

    return res;
}

}    // namespace Proto
