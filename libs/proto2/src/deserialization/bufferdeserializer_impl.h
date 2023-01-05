#pragma once

#include "bufferdeserializer.h"

#include <cassert>

namespace Proto2
{

template <std::integral T>
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

    return std::string{ mem, leftover };
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
        return std::string{ mem, size };
}

}    // namespace Proto2
