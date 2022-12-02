#pragma once

#include "proto/deserialization.h"

#include <cstring>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "utils/copymove.h"

namespace Proto::detail
{
struct DeserializationPtr
{
    DISABLE_COPY_MOVE(DeserializationPtr);

    DeserializationPtr(const void *data, size_t size)
        : data_{ data }, left_{ data_ == nullptr ? 0 : size }
    {
    }

    /**
     * @brief tries to read the next @p count objects
     * of type @tparam T from the buffer. Returns nullptr
     * if we overflow the buffer
     *
     * @tparam T
     * @return const T*
     */
    template <typename T>
    const T *consume(size_t count = 1)
    {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        static_assert(std::is_pod_v<T>);

        if (count == 0)
            return nullptr;

        if (left_ < sizeof(T) * count)
        {
            return nullptr;
        }
        else
        {
            auto res = static_cast<const T *>(data_);
            left_ -= sizeof(T) * count;
            data_ = static_cast<const std::byte *>(data_) + sizeof(T) * count;
            return res;
        }
    }

    bool atEnd() const noexcept { return left_ == 0; }

private:
    const void *data_;
    size_t      left_;
};

template <typename T>
class Deserializer
{
public:
    static std::optional<T> deserialize(DeserializationPtr &);
};

template <typename T>
class Deserializer<std::optional<T>>
{
public:
    static std::optional<std::optional<T>> deserialize(DeserializationPtr &);
};

template <typename T>
class Deserializer<std::vector<T>>
{
public:
    static std::optional<std::vector<T>> deserialize(DeserializationPtr &);
};

template <>
class Deserializer<std::string>
{
public:
    static std::optional<std::string> deserialize(DeserializationPtr &);
};

template <typename T>
std::optional<T> Deserializer<T>::deserialize(DeserializationPtr &ptr)
{
    static_assert(std::is_same_v<T, std::decay_t<T>>);

    if constexpr (std::is_pod_v<T>)
    {
        if (const auto data = ptr.consume<T>())
            return *data;
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
std::optional<std::optional<T>>
    Deserializer<std::optional<T>>::deserialize(DeserializationPtr &ptr)
{
    const auto presentPtr = ptr.consume<bool>();

    if (presentPtr == nullptr)
        return {};

    const auto present = *presentPtr;

    if (!present)
        return std::optional<T>{};

    auto optRes = Deserializer<T>::deserialize(ptr);
    if (optRes.has_value())
        return std::optional<std::optional<T>>{ std::move(optRes.value()) };
    else
        return {};
}

template <typename T>
std::optional<std::vector<T>>
    Deserializer<std::vector<T>>::deserialize(DeserializationPtr &ptr)
{
    const auto sizePtr = ptr.consume<size_t>();
    if (sizePtr == nullptr)
        return {};

    const auto size = *sizePtr;
    auto       res  = std::vector<T>{};
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

}    // namespace Proto::detail

namespace Proto
{
template <typename Event>
[[nodiscard]] std::optional<Event> deserialize(const void *data, size_t size)
{
    auto ptr = detail::DeserializationPtr{ data, size };
    auto res = detail::Deserializer<Event>::deserialize(ptr);

    if (!ptr.atEnd())
        return {};
    else
        return res;
}

}    // namespace Proto
