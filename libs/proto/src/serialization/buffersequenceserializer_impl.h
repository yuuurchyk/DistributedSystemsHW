#pragma once

#include "buffersequenceserializer.h"

namespace Proto
{
template <typename T>
    requires std::is_integral_v<T> || std::is_enum_v<T>
void BufferSequenceSerializer::serializeWoOwnership(const T &val)
{
    serializeWoOwnership(reinterpret_cast<const std::byte *>(&val), sizeof(T));
}

template <typename T>
    requires std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>
void BufferSequenceSerializer::serializeWoOwnership(const T &val)
{
    static_assert(sizeof(typename T::value_type) == 1);

    serializeWoOwnership(reinterpret_cast<const std::byte *>(val.data()), val.size());
}

}    // namespace Proto
