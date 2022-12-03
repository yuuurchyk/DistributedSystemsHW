#pragma once

#include "proto/deserialization.h"

#include <algorithm>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/asio.hpp>

#include "utils/copymove.h"

namespace Proto::detail
{
template <typename BuffersIterator>
struct DeserializationPtr
{
    DISABLE_COPY_MOVE(DeserializationPtr);

    DeserializationPtr(BuffersIterator begin, BuffersIterator end)
        : current{ begin }, end{ end }
    {
    }

    /**
     * @return true if buffer has @p count more instances of T
     */
    template <typename T>
    bool has(size_t count = 1)
    {
        const auto requiredSize = sizeof(T) * count;
        return (end - current) >= requiredSize;
    }

    template <typename T>
    T get()
    {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        static_assert(std::is_pod_v<T>);

        T res;
        std::copy(current,
                  current + sizeof(T),
                  reinterpret_cast<typename BuffersIterator::value_type *>(&res));

        current += sizeof(T);

        return res;
    }

    template <typename T>
    std::tuple<BuffersIterator, BuffersIterator> get(size_t count)
    {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        static_assert(std::is_pod_v<T>);

        auto res = std::make_tuple(current, current + sizeof(T) * count);
        current += sizeof(T) * count;

        return res;
    }

    bool atEnd() const noexcept { return current == end; }

private:
    BuffersIterator       current;
    const BuffersIterator end;
};

template <typename T>
class Deserializer
{
public:
    template <typename BuffersIterator>
    static std::optional<T> deserialize(DeserializationPtr<BuffersIterator> &);
};

template <typename T>
class Deserializer<std::optional<T>>
{
public:
    template <typename BuffersIterator>
    static std::optional<std::optional<T>>
        deserialize(DeserializationPtr<BuffersIterator> &);
};

template <typename T>
class Deserializer<std::vector<T>>
{
public:
    template <typename BuffersIterator>
    static std::optional<std::vector<T>>
        deserialize(DeserializationPtr<BuffersIterator> &);
};

template <>
class Deserializer<std::string>
{
public:
    template <typename BuffersIterator>
    static std::optional<std::string> deserialize(DeserializationPtr<BuffersIterator> &);
};

template <typename T>
template <typename BuffersIterator>
std::optional<T> Deserializer<T>::deserialize(DeserializationPtr<BuffersIterator> &ptr)
{
    static_assert(std::is_same_v<T, std::decay_t<T>>);

    if constexpr (std::is_pod_v<T>)
    {
        if (ptr.template has<T>())
            return ptr.template get<T>();
        else
            return {};
    }
    else
    {
        auto allGood = true;

        auto res   = T{};
        auto tuple = res.tie();

        auto visit = [&](auto &target)
        {
            if (!allGood)
                return;

            auto res = Deserializer<std::decay_t<decltype(target)>>::deserialize(ptr);
            if (res.has_value())
                target = std::move(res.value());
            else
                allGood = false;
        };

        std::apply([&](auto &...val) { (..., visit(val)); }, tuple);

        if (allGood)
            return res;
        else
            return {};
    }
}

template <typename T>
template <typename BuffersIterator>
std::optional<std::optional<T>>
    Deserializer<std::optional<T>>::deserialize(DeserializationPtr<BuffersIterator> &ptr)
{
    if (!ptr.template has<bool>())
        return {};

    const auto present = ptr.template get<bool>();

    if (!present)
        return std::optional<T>{};

    auto optRes = Deserializer<T>::deserialize(ptr);
    if (optRes.has_value())
        return std::optional<std::optional<T>>{ std::move(optRes.value()) };
    else
        return {};
}

template <typename T>
template <typename BuffersIterator>
std::optional<std::vector<T>>
    Deserializer<std::vector<T>>::deserialize(DeserializationPtr<BuffersIterator> &ptr)
{
    if (!ptr.template has<size_t>())
        return {};

    const auto size = ptr.template get<size_t>();

    auto res = std::vector<T>{};
    res.reserve(size);

    for (auto i = size_t{}; i < size; ++i)
    {
        auto optRes = Deserializer<T>::deserialize(ptr);
        if (!optRes.has_value())
            return {};
        else
            res.push_back(std::move(optRes.value()));
    }

    return res;
}

template <typename BuffersIterator>
std::optional<std::string>
    Deserializer<std::string>::deserialize(DeserializationPtr<BuffersIterator> &ptr)
{
    static_assert(std::is_pod_v<std::string::value_type>);

    if (!ptr.template has<size_t>())
        return {};

    const auto size = ptr.template get<size_t>();

    auto res = std::string{};
    res.reserve(size);

    if (!ptr.template has<std::string::value_type>(size))
        return {};

    const auto [l, r] = ptr.template get<std::string::value_type>(size);

    res.insert(res.begin(), l, r);

    return res;
}

}    // namespace Proto::detail

namespace Proto
{
template <typename Event, typename ConstBufferSequence>
[[nodiscard]] std::optional<Event> deserialize(const ConstBufferSequence &seq)
{
    auto begin = boost::asio::buffers_begin(seq);
    auto end   = boost::asio::buffers_end(seq);

    auto ptr = detail::DeserializationPtr{ begin, end };
    auto res = detail::Deserializer<Event>::deserialize(ptr);

    if (!ptr.atEnd())
        return {};
    else
        return res;
}

}    // namespace Proto
