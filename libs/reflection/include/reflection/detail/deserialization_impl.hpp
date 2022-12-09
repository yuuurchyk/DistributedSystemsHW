#pragma once

#include "reflection/deserialization.h"

namespace Reflection
{
template <typename ConstBufferSequence>
template <Deserializable T>
std::optional<T> DeserializationContext<ConstBufferSequence>::deserialize()
{
    auto currentCpy = current_;

    auto res = deserializaImpl(Tag<T>{});

    if (!res.has_value())
        current_ = currentCpy;

    return res;
}

template <typename ConstBufferSequence>
DeserializationContext<ConstBufferSequence>::DeserializationContext(
    const ConstBufferSequence &seq)
    : current_{ boost::asio::buffers_begin(seq) }, end_{ boost::asio::buffers_end(seq) }
{
}

template <typename ConstBufferSequence>
template <TriviallyDeserializable T>
bool DeserializationContext<ConstBufferSequence>::has(size_t count)
{
    const auto requiredSize = sizeof(T) * count;
    return (end_ - current_) >= requiredSize;
}

template <typename ConstBufferSequence>
template <TriviallyDeserializable T>
T DeserializationContext<ConstBufferSequence>::get()
{
    T res;
    std::copy(current_,
              current_ + sizeof(T),
              reinterpret_cast<typename Iterator::value_type *>(&res));

    current_ += sizeof(T);

    return res;
}

template <typename ConstBufferSequence>
template <TriviallyDeserializable T>
auto DeserializationContext<ConstBufferSequence>::get(size_t count)
    -> std::tuple<Iterator, Iterator>
{
    auto res = std::make_tuple(current_, current_ + sizeof(T) * count);
    current_ += sizeof(T) * count;

    return res;
}

template <typename ConstBufferSequence>
bool DeserializationContext<ConstBufferSequence>::atEnd() const noexcept
{
    return current_ == end_;
}

// ---------------------------------------------------------

template <typename ConstBufferSequence>
template <TriviallyDeserializable T>
std::optional<T> DeserializationContext<ConstBufferSequence>::deserializeImpl(Tag<T>)
{
    if (!has<T>())
        return {};
    return get<T>();
}

template <typename ConstBufferSequence>
template <HasDeserializableTie T>
std::optional<T> DeserializationContext<ConstBufferSequence>::deserializeImpl(Tag<T>)
{
    auto allGood = true;

    auto res = T{};

    auto visit = [&](auto &target)
    {
        if (!allGood)
            return;

        auto res = deserializeImpl(Tag<std::decay_t<decltype(target)>>{});

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

template <typename ConstBufferSequence>
template <TriviallyDeserializable T>
std::optional<std::vector<T>>
    DeserializationContext<ConstBufferSequence>::deserializeImpl(Tag<std::vector<T>>)
{
    const auto optSize = deserializeImpl(Tag<typename std::vector<T>::size_type>{});

    if (!optSize.has_value())
        return {};
    const auto size = optSize.value();

    if (!has<T>(size))
        return {};

    const auto [l, r] = get<T>(size);

    auto res = std::vector<T>{};
    res.reserve(size);
    res.insert(res.begin(), l, r);

    return res;
}

template <typename ConstBufferSequence>
template <NonTriviallyDeserializable T>
std::optional<std::vector<T>>
    DeserializationContext<ConstBufferSequence>::deserializeImpl(Tag<std::vector<T>>)
{
    const auto optSize = deserializeImpl(Tag<typename std::vector<T>::size_type>{});

    if (!optSize.has_value())
        return {};
    const auto size = optSize.value();

    auto res = std::vector<T>{};
    res.reserve(size);

    for (auto i = size_t{}; i < size; ++i)
    {
        auto optEntry = deserializeImpl(Tag<T>{});
        if (!optEntry.has_value())
            return {};
        else
            res.push_back(std::move(optEntry.value()));
    }

    return res;
}

template <typename ConstBufferSequence>
std::optional<std::string>
    DeserializationContext<ConstBufferSequence>::deserializeImpl(Tag<std::string>)
{
    if (!has<std::string::size_type>())
        return {};

    const auto size = get<std::string::size_type>();

    auto res = std::string{};
    res.reserve(size);

    if (!has<std::string::value_type>(size))
        return {};

    const auto [l, r] = get<std::string::value_type>(size);

    res.insert(res.begin(), l, r);

    return res;
}

}    // namespace Reflection
