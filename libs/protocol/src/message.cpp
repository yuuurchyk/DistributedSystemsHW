#include "protocol/message.h"

#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace protocol
{
const Message::size_type Message::kInvalidMessage{};

Message::Message()
{
    rawMemory_.reset(new (std::nothrow) std::byte[sizeof(size_type)]);
    if (rawMemory_ == nullptr)
    {
        invalidate();
    }
    else
    {
        *reinterpret_cast<size_type *>(rawMemory_.get()) = 0;

        size_     = sizeof(size_type);
        capacity_ = size_;
    }
}

void Message::reserveSpaceFor(size_t bytesNum)
{
    if (wasInvalidated_)
        return;

    const auto left = capacity_ - size_;
    if (left >= bytesNum)
        return;

    const auto needToAdditionallyAllocate = bytesNum - left;

    // capacity_ + needToAdditionallyAllocate <= std::numeric_limits::max()
    if (needToAdditionallyAllocate > std::numeric_limits<size_t>::max() - capacity_)
        return invalidate();

    auto newMemory = std::unique_ptr<std::byte[]>{ new (
        std::nothrow) std::byte[capacity_ + needToAdditionallyAllocate] };
    if (newMemory == nullptr)
        return invalidate();

    std::memcpy(newMemory.get(), rawMemory_.get(), size_);
    rawMemory_.swap(newMemory);
    capacity_ += needToAdditionallyAllocate;
}

bool Message::valid() const noexcept
{
    return !wasInvalidated_ && payloadSize() > 0;
}

size_t Message::size() const noexcept
{
    return size_;
}

const std::byte *Message::message() const noexcept
{
    return valid() ? rawMemory_.get() :
                     reinterpret_cast<const std::byte *>(&kInvalidMessage);
}

size_t Message::payloadSize() const noexcept
{
    return payloadSize_;
}

const std::byte *Message::payload() const noexcept
{
    return valid() ? rawMemory_.get() + sizeof(size_type) : nullptr;
}

void Message::invalidate()
{
    if (wasInvalidated_)
        return;

    wasInvalidated_ = true;
    rawMemory_.reset();
    size_        = sizeof(size_type);
    payloadSize_ = 0;
}

void Message::addMemoryChunk(const std::byte *start, size_t size)
{
    reserveSpaceFor(size);

    if (wasInvalidated_)
        return;

    std::memcpy(rawMemory_.get() + size_, start, size);
    size_ += size;
    payloadSize_ += size;
    *reinterpret_cast<size_type *>(rawMemory_.get()) += size;
}

}    // namespace protocol
