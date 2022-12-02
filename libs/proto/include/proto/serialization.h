#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include <boost/asio.hpp>

#include "utils/copymove.h"

namespace Proto
{
class SerializationContext;

template <typename Event>    // for both Request and Response
[[nodiscard]] std::unique_ptr<SerializationContext> serialize(Event);

class SerializationContext
{
    DISABLE_COPY_MOVE(SerializationContext)

public:
    std::deque<boost::asio::const_buffer> constBufferSequence;

    /**
     * @brief saves pod on the heap
     */
    template <typename T, typename = std::enable_if_t<std::is_pod_v<T>>>
    const T &add(T value);

    SerializationContext() = default;
    ~SerializationContext();

private:
    template <typename Event>
    friend std::unique_ptr<SerializationContext> serialize(Event);

    std::vector<std::function<void()>> dtors_;    // wierd way of holding arbitrary memory
                                                  // chunks allocated on the heap
};

}    // namespace Proto

#include "detail/serialization_impl.hpp"
