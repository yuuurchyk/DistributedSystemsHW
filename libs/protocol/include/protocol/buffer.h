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

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    Buffer &operator<<(T val);

    template <typename Str,
              typename = std::enable_if_t<std::is_same_v<Str, std::string> ||
                                          std::is_same_v<Str, std::string_view>>>
    Buffer &operator<<(const Str &s);

    size_t           size() const noexcept;
    const std::byte *data() const noexcept;

private:
    void invalidate();
    void addMemoryChunk(const std::byte *start, size_t size);

    bool                         invalidated_{ false };
    std::unique_ptr<std::byte[]> rawMemory_{};
    size_t                       size_{};
    size_t                       capacity_{};
};

}    // namespace protocol

template <typename T, typename>
auto protocol::Buffer::operator<<(T val) -> Buffer &
{
    static_assert(std::is_integral_v<T>, "only for integral types");
    addMemoryChunk(reinterpret_cast<const std::byte *>(&val), sizeof(val));
    return *this;
}

template <typename Str, typename>
auto protocol::Buffer::operator<<(const Str &s) -> Buffer &
{
    static_assert(std::is_same_v<Str, std::string> ||
                  std::is_same_v<Str, std::string_view>);

    addMemoryChunk(reinterpret_cast<const std::byte *>(s.data()),
                   s.size() * sizeof(char));

    return *this;
}
