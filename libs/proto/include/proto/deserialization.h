#pragma once

#include <cstddef>
#include <optional>

namespace Proto
{
template <typename Event, typename ConstBufferSequence>
[[nodiscard]] std::optional<Event> deserialize(const ConstBufferSequence &);

}    // namespace Proto

#include "detail/deserialization_impl.hpp"
