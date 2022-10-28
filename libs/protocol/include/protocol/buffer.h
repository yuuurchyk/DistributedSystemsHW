#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace protocol
{
class Buffer
{
public:
    Buffer();
    Buffer(size_t capacity);

    Buffer(const Buffer &)            = delete;
    Buffer(Buffer &&)                 = default;
    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&)      = default;

    ~Buffer() = default;

    /**
     * @brief whether we could not allocated memory
     * for new data at some point
     */
    bool invalidated() const noexcept;

    /**
     * @brief reserves space for at least @p bytesNum more bytes
     */
    void reserveSpaceFor(size_t bytesNum);

    template <typename T,
              typename = std::enable_if_t<std::is_integral_v<T> || std::is_enum_v<T>>>
    Buffer &operator<<(T val);

    template <typename Str,
              typename = std::enable_if_t<std::is_same_v<Str, std::string> ||
                                          std::is_same_v<Str, std::string_view>>>
    Buffer &operator<<(const Str &s);

    size_t           size() const noexcept;
    std::byte       *data() noexcept;
    const std::byte *data() const noexcept;

private:
    void invalidate();
    void addMemoryChunk(const std::byte *start, size_t size);

    bool                         invalidated_{ false };
    std::unique_ptr<std::byte[]> rawMemory_{};
    size_t                       size_{};
    size_t                       capacity_{};
};

class BufferView
{
public:
    BufferView(const std::byte *data, size_t size);

    const std::byte *data() const noexcept;
    size_t           size() const noexcept;

private:
    const std::byte *data_;
    size_t           size_;
};

}    // namespace protocol

template <typename T, typename>
auto protocol::Buffer::operator<<(T val) -> Buffer &
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>,
                  "only for integral and enum types");
    static_assert(std::is_same_v<T, std::decay_t<T>>);

    addMemoryChunk(reinterpret_cast<const std::byte *>(&val), sizeof(val));

    return *this;
}

template <typename Str, typename>
auto protocol::Buffer::operator<<(const Str &s) -> Buffer &
{
    static_assert(std::is_same_v<Str, std::string> ||
                  std::is_same_v<Str, std::string_view>);
    static_assert(std::is_same_v<Str, std::decay_t<Str>>);

    addMemoryChunk(reinterpret_cast<const std::byte *>(s.data()),
                   s.size() * sizeof(char));

    return *this;
}
