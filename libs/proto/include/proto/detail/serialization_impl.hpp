#pragma once

#include "proto/concepts.h"
#include "proto/detail/traits.h"
#include "proto/serialization.h"

#include <cstring>

#include <optional>
#include <string>
#include <utility>

namespace Proto::detail::Serialization
{
template <Concepts::TriviallySerializable T>
[[nodiscard]] size_t buffersRequired(const T &);
template <Concepts::TriviallySerializable T>
[[nodiscard]] size_t additionalBytesRequired(const T &);
template <Concepts::TriviallySerializable T>
void serialize(const T &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);

template <Concepts::HasSerializableTie T>
[[nodiscard]] size_t buffersRequired(const T &);
template <Concepts::HasSerializableTie T>
[[nodiscard]] size_t additionalBytesRequired(const T &);
template <Concepts::HasSerializableTie T>
void serialize(const T &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);

template <Concepts::TriviallySerializable T>
[[nodiscard]] size_t buffersRequired(const std::vector<T> &);
template <Concepts::TriviallySerializable T>
[[nodiscard]] size_t additionalBytesRequired(const std::vector<T> &);
template <Concepts::TriviallySerializable T>
void serialize(const std::vector<T> &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);

template <Concepts::NonTriviallySerializable T>
[[nodiscard]] size_t buffersRequired(const std::vector<T> &);
template <Concepts::NonTriviallySerializable T>
[[nodiscard]] size_t additionalBytesRequired(const std::vector<T> &);
template <Concepts::NonTriviallySerializable T>
void serialize(const std::vector<T> &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);

template <Concepts::Serializable T>
[[nodiscard]] size_t buffersRequired(const std::optional<T> &);
template <Concepts::Serializable T>
[[nodiscard]] size_t additionalBytesRequired(const std::optional<T> &);
template <Concepts::Serializable T>
void serialize(const std::optional<T> &,
               std::vector<boost::asio::const_buffer> &,
               SerializationContext &);

static_assert(Concepts::Serializable<std::string>);
[[nodiscard]] size_t buffersRequired(const std::string &);
[[nodiscard]] size_t additionalBytesRequired(const std::string &);
void                 serialize(const std::string &,
                               std::vector<boost::asio::const_buffer> &,
                               SerializationContext &);

// ---------------------------------------------------------

template <Concepts::TriviallySerializable T>
size_t buffersRequired(const T &)
{
    return 1;
}
template <Concepts::TriviallySerializable T>
size_t additionalBytesRequired(const T &)
{
    return 0;
}
template <Concepts::TriviallySerializable T>
void serialize(const T                                &val,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext &)
{
    seq.emplace_back(static_cast<const void *>(&val), sizeof(val));
}

// ---------------------------------------------------------

template <Concepts::HasSerializableTie T>
size_t buffersRequired(const T &val)
{
    auto res = size_t{};

    auto visit = [&](const auto &val) { res += buffersRequired(val); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, val.tie());

    return res;
}

template <Concepts::HasSerializableTie T>
size_t additionalBytesRequired(const T &val)
{
    auto res = size_t{};

    auto visit = [&](const auto &val) { res += additionalBytesRequired(val); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, val.tie());

    return res;
}

template <Concepts::HasSerializableTie T>
void serialize(const T                                &val,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    auto visit = [&](const auto &entry) { serialize(entry, seq, context); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, val.tie());
}

// ---------------------------------------------------------

template <Concepts::TriviallySerializable T>
size_t buffersRequired(const std::vector<T> &v)
{
    return 2;    // size + entries
}

template <Concepts::TriviallySerializable T>
size_t additionalBytesRequired(const std::vector<T> &v)
{
    return sizeof(std::vector<T>::size_type);
}

template <Concepts::TriviallySerializable T>
void serialize(const std::vector<T>                   &v,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    serialize(context.add(v.size()), seq, context);
    seq.emplace_back(_cast<const void *>(v.data()), v.size() * sizeof(T));
}

// ---------------------------------------------------------

template <Concepts::NonTriviallySerializable T>
size_t buffersRequired(const std::vector<T> &v)
{
    auto res = size_t{ 1 };
    for (const auto &entry : v)
        res += buffersRequired(entry);
    return res;
}

template <Concepts::NonTriviallySerializable T>
size_t additionalBytesRequired(const std::vector<T> &v)
{
    auto res = size_t{ sizeof(typename std::vector<T>::size_type) };

    for (const auto &entry : v)
        res += additionalBytesRequired(entry);

    return res;
}

template <Concepts::NonTriviallySerializable T>
void serialize(const std::vector<T>                   &v,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    serialize(context.add(v.size()), seq, context);

    for (const auto &entry : v)
        serialize(entry, seq, context);
}

// ---------------------------------------------------------

template <Concepts::Serializable T>
size_t buffersRequired(const std::optional<T> &opt)
{
    auto res = size_t{ 1 };

    if (opt.has_value())
        res += buffersRequired(opt.value());

    return res;
}

template <Concepts::Serializable T>
size_t additionalBytesRequired(const std::optional<T> &opt)
{
    auto res = size_t{ sizeof(bool) };

    if (opt.has_value())
        res += additionalBytesRequired(opt.value());

    return res;
}

template <Concepts::Serializable T>
void serialize(const std::optional<T>                 &opt,
               std::vector<boost::asio::const_buffer> &seq,
               SerializationContext                   &context)
{
    serialize(context.add(opt.has_value()), seq, context);

    if (opt.has_value())
        serialize(opt.value(), seq, context);
}

}    // namespace Proto::detail::Serialization

namespace Proto
{
template <Concepts::TriviallySerializable T>
const T &SerializationContext::add(const T &value)
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
