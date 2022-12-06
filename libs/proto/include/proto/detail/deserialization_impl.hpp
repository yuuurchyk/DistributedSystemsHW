#pragma once

#include "proto/deserialization.h"

namespace Proto
{
template <typename ConstBufferSequence>
DeserializationPtr<ConstBufferSequence>::DeserializationPtr(
    const ConstBufferSequence &seq)
    : current_{ boost::asio::buffers_begin(seq) }, end_{ boost::asio::buffers_end(seq) }
{
}

template <typename ConstBufferSequence>
template <Concepts::TriviallyDeserializable T>
bool DeserializationPtr<ConstBufferSequence>::has(size_t count)
{
    const auto requiredSize = sizeof(T) * count;
    return (end_ - current_) >= requiredSize;
}

template <typename ConstBufferSequence>
template <Concepts::TriviallyDeserializable T>
T DeserializationPtr<ConstBufferSequence>::get()
{
    T res;
    std::copy(current_,
              current_ + sizeof(T),
              reinterpret_cast<typename Iterator::value_type *>(&res));

    current_ += sizeof(T);

    return res;
}

template <typename ConstBufferSequence>
template <Concepts::TriviallyDeserializable T>
auto DeserializationPtr<ConstBufferSequence>::get(size_t count)
    -> std::tuple<Iterator, Iterator>
{
    auto res = std::make_tuple(current_, current_ + sizeof(T) * count);
    current_ += sizeof(T) * count;

    return res;
}

template <typename ConstBufferSequence>
bool DeserializationPtr<ConstBufferSequence>::atEnd() const noexcept
{
    return current_ == end_;
}

}    // namespace Proto

namespace Proto::detail::Deserialization
{
template <Concepts::Deserializable T>
struct Tag
{
};

template <Concepts::TriviallyDeserializable T, typename ConstBufferSequence>
[[nodiscard]] std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &,
                                           Tag<T>);
template <Concepts::HasDeserializableTie T, typename ConstBufferSequence>
[[nodiscard]] std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &,
                                           Tag<T>);

template <Concepts::TriviallyDeserializable T, typename ConstBufferSequence>
[[nodiscard]] std::optional<std::vector<T>>
    deserialize(DeserializationPtr<ConstBufferSequence> &, Tag<std::vector<T>>);
template <Concepts::NonTriviallyDeserializable T, typename ConstBufferSequence>
[[nodiscard]] std::optional<std::vector<T>>
    deserialize(DeserializationPtr<ConstBufferSequence> &, Tag<std::vector<T>>);

template <Concepts::Deserializable T, typename ConstBufferSequence>
[[nodiscard]] std::optional<std::optional<T>>
    deserialize(DeserializationPtr<ConstBufferSequence> &, Tag<std::optional<T>>);

template <typename ConstBufferSequence>
[[nodiscard]] std::optional<std::string>
    deserialize(DeserializationPtr<ConstBufferSequence> &, Tag<std::string>);

// ---------------------------------------------------------

template <Concepts::TriviallyDeserializable T, typename ConstBufferSequence>
std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &ptr, Tag<T>)
{
    if (!ptr.template has<T>())
        return {};
    return ptr.template get<T>();
}

template <Concepts::HasDeserializableTie T, typename ConstBufferSequence>
std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &ptr, Tag<T>)
{
    auto allGood = true;

    auto res = T{};

    auto visit = [&](auto &target)
    {
        if (!allGood)
            return;

        auto res = deserialize(ptr, Tag<std::decay_t<decltype(target)>>{});

        if (res.has_value())
            target = std::move(res.value());
        else
            allGood = false;
    };

    std::apply([&](auto &...val) { (..., visit(val)); }, res.tie());

    if (allGood)
        return res;
    else
        return {};
}

template <Concepts::TriviallyDeserializable T, typename ConstBufferSequence>
[[nodiscard]] std::optional<std::vector<T>>
    deserialize(DeserializationPtr<ConstBufferSequence> &ptr, Tag<std::vector<T>>)
{
    const auto optSize = deserialize(ptr, Tag<typename std::vector<T>::size_type>{});

    if (!optSize.has_value())
        return {};
    const auto size = optSize.value();

    if (!ptr.template has<T>(size))
        return {};

    const auto [l, r] = ptr.template get<T>(size);

    auto res = std::vector<T>{};
    res.reserve(size);
    res.insert(res.begin(), l, r);

    return res;
}

template <Concepts::NonTriviallyDeserializable T, typename ConstBufferSequence>
[[nodiscard]] std::optional<std::vector<T>>
    deserialize(DeserializationPtr<ConstBufferSequence> &ptr, Tag<std::vector<T>>)
{
    const auto optSize = deserialize(ptr, Tag<typename std::vector<T>::size_type>{});

    if (!optSize.has_value())
        return {};
    const auto size = optSize.value();

    auto res = std::vector<T>{};
    res.reserve(size);

    for (auto i = size_t{}; i < size; ++i)
    {
        auto optEntry = deserialize(ptr, Tag<T>{});
        if (!optEntry.has_value())
            return {};
        else
            res.push_back(std::move(optEntry.value()));
    }

    return res;
}

template <Concepts::Deserializable T, typename ConstBufferSequence>
std::optional<std::optional<T>> deserialize(DeserializationPtr<ConstBufferSequence> &ptr,
                                            Tag<std::optional<T>>)
{
    if (!ptr.template has<bool>())
        return {};

    const auto present = ptr.template get<bool>();

    if (!present)
        return std::optional<T>{};

    auto optRes = deserialize(ptr, Tag<T>{});
    if (optRes.has_value())
        return std::optional<std::optional<T>>{ std::move(optRes.value()) };
    else
        return {};
}

template <typename ConstBufferSequence>
std::optional<std::string> deserialize(DeserializationPtr<ConstBufferSequence> &ptr,
                                       Tag<std::string>)
{
    if (!ptr.template has<std::string::size_type>())
        return {};

    const auto size = ptr.template get<std::string::size_type>();

    auto res = std::string{};
    res.reserve(size);

    if (!ptr.template has<std::string::value_type>(size))
        return {};

    const auto [l, r] = ptr.template get<std::string::value_type>(size);

    res.insert(res.begin(), l, r);

    return res;
}

}    // namespace Proto::detail::Deserialization

namespace Proto
{
template <Concepts::Deserializable T, typename ConstBufferSequence>
std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &ptr)
{
    return detail::Deserialization::deserialize(ptr, detail::Deserialization::Tag<T>{});
}

}    // namespace Proto
