#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "protocol/buffer.h"
#include "protocol/codes.h"

namespace protocol
{
/**
 * @brief Structure of Frame:
 * 8 bytes: frame size
 * 1 byte : codes::Event
 * 1 byte : codes::OpCode
 * 8 bytes: request id
 * ...      body
 */
class Frame
{
public:
    using size_type = uint64_t;
    static_assert(sizeof(size_type) >= sizeof(size_t));

    Frame(Buffer);

    Frame(const Frame &)            = delete;
    Frame(Frame &&)                 = default;
    Frame &operator=(const Frame &) = delete;
    Frame &operator=(Frame &&)      = default;

    ~Frame() = default;

    /**
     * @note frame body should be checked separately
     */
    bool valid() const noexcept;

    codes::Event  event() const;
    codes::OpCode opCode() const;
    size_type     requestId() const;

    BufferView body() const;
    Buffer     flushBuffer();

protected:
    void invalidate();

private:
    static constexpr size_t kFrameSizeOffset{ 0 };
    static constexpr size_t kEventOffset{ kFrameSizeOffset + sizeof(size_type) };
    static constexpr size_t kOpCodeOffset{ kEventOffset + sizeof(codes::Event) };
    static constexpr size_t kRequestIdOffset{ kOpCodeOffset + sizeof(codes::OpCode) };
    static constexpr size_t kBodyOffset{ kRequestIdOffset + sizeof(size_type) };

    void decideOnValidity();

    Buffer buffer_;
    bool   valid_{ true };
};

}    // namespace protocol
