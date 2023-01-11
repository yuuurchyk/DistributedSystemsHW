#pragma once

#include <concepts>
#include <optional>
#include <string>
#include <type_traits>

#include <boost/asio.hpp>

namespace Proto2
{
class BufferDeserializer
{
public:
    BufferDeserializer(boost::asio::const_buffer);

    /**
     * @return bool - whether there is nothing to read from the buffer
     */
    bool atEnd() const;

    /**
     * @return boost::asio::const_buffer - new buffer that is the last
     * part of the initial buffer that hasn't been read
     */
    boost::asio::const_buffer leftover() const;

    template <typename T>
        requires std::is_integral_v<T> || std::is_enum_v<T>
    std::optional<T> deserialize();

    /**
     * @brief treats all remaining buffer characters as a string.
     *
     * Resulting std::optional has value in case the underlying buffer
     * is not nullptr
     */
    template <typename T>
        requires std::is_same_v<T, std::string>
    std::optional<std::string> deserialize();

    /**
     * @brief treats next size characters as a string
     *
     * Resulting std::optional has value in case there is enough
     * leftover characters in the buffer
     */
    template <typename T>
        requires std::is_same_v<T, std::string>
    std::optional<std::string> deserialize(size_t size);

private:
    const std::byte *getMemoryAndAdvance(size_t bytes);

    const std::byte *const begin_{};
    const std::byte *const end_{};

    const std::byte *current_{};
};

}    // namespace Proto2

#include "bufferdeserializer_impl.h"
