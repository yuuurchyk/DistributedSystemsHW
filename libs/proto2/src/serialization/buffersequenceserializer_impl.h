#pragma once

#include "buffersequenceserializer.h"

namespace Proto2
{
template <std::integral T>
void BufferSequenceSerializer::serialize(const T *val)
{
    serialize(reinterpret_cast<const std::byte *>(val), sizeof(T));
}

template <typename T>
    requires std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>
void BufferSequenceSerializer::serialize(const T *val)
{
    static_assert(sizeof(typename T::value_type) == 1);

    if (val == nullptr)
        return;
    else
        serialize(reinterpret_cast<const std::byte *>(val->data()), val->size());
}

}    // namespace Proto2
