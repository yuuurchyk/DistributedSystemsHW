#pragma once

#include <cstddef>
#include <optional>

namespace Proto
{
template <typename Event>
[[nodiscard]] std::optional<Event> deserialize(const void *data, size_t size);

}    // namespace Proto

#include "detail/deserialization_impl.hpp"
