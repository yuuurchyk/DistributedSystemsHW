#pragma once

#include "bufferdeserializer.h"

#include <cassert>

namespace Proto2
{
template <typename T>
    requires std::is_integral_v<T> || std::is_enum_v<T>
std::optional<T> BufferDeserializer::deserialize()
{
    const auto mem = getMemoryAndAdvance(sizeof(T));

    if (mem == nullptr)
        return {};
    else
        return *reinterpret_cast<const T *>(mem);
}

template <typename T>
    requires std::is_same_v<T, std::string>
std::optional<std::string> BufferDeserializer::deserialize()
{
    static_assert(sizeof(std::string::value_type) == 1);

    if (begin_ == nullptr)
        return {};

    const auto leftover = end_ - current_;

    const auto mem = getMemoryAndAdvance(leftover);
    if (mem == nullptr)
    {
        assert(false);
        return {};
    }

    return std::string{ reinterpret_cast<const char *>(mem), static_cast<size_t>(leftover) };
}

template <typename T>
    requires std::is_same_v<T, std::string>
std::optional<std::string> BufferDeserializer::deserialize(size_t size)
{
    static_assert(sizeof(std::string::value_type) == 1);

    const auto mem = getMemoryAndAdvance(size);
    if (mem == nullptr)
        return {};
    else
        return std::string{ reinterpret_cast<const char *>(mem), size };
}

}    // namespace Proto2
