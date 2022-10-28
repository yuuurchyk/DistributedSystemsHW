#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace protocol
{
/**
 * @brief Generate message: 8 bytes (for payload size) + payload itself
 *
 * @note output operators (<<) automatically handle changing the size of the payload
 *
 * Intended usage:
 *
 * @code
 * auto m = Message{};
 * m << 1 << 2 << 3 << "hello world";
 * @endcode
 *
 * In order for the previous example to be more productive, reserve beforehand:
 * @code
 * auto m = Message{};
 * m.reserveSpaceFor(sizeof(int) * 3 + 11);
 * m << 1 << 2 << 3 << "hello world";
 * @endcode
 */
class Message
{
public:
    using size_type = uint64_t;

    Message();

    Message(const Message &)            = delete;
    Message(Message &&)                 = default;
    Message &operator=(const Message &) = delete;
    Message &operator=(Message &&)      = default;

    ~Message() = default;

    /**
     * @brief reserves space for at least @p bytesNum more bytes
     */
    void reserveSpaceFor(size_t bytesNum);

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    Message &operator<<(T val);

    Message &operator<<(const std::string &s);
    Message &operator<<(const std::string_view &s);

    bool valid() const noexcept;

    size_t           size() const noexcept;
    const std::byte *message() const noexcept;

    // nullptr when valid() = false
    size_t           payloadSize() const noexcept;
    const std::byte *payload() const noexcept;

private:
    void invalidate();
    void addMemoryChunk(const std::byte *start, size_t size);

    // guaranteed chunk of memory for invalid messages
    static const size_type kInvalidMessage;

    bool wasInvalidated_{ false };

    std::unique_ptr<std::byte[]> rawMemory_{};
    size_t                       size_{ sizeof(size_type) };
    size_t                       capacity_{};
    size_t                       payloadSize_{};
};

}    // namespace protocol

template <typename T, typename>
auto protocol::Message::operator<<(T val) -> Message &
{
    static_assert(std::is_integral_v<T>, "only for integral types");
    addMemoryChunk(reinterpret_cast<const std::byte *>(&val), sizeof(val));
    return *this;
}
