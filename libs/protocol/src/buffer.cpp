#include "protocol/buffer.h"

#include <cstring>
#include <limits>

namespace protocol
{

Buffer::Buffer()
{
    rawMemory_.reset(new (std::nothrow) std::byte[1]);
    if (rawMemory_.get() == nullptr)
    {
        invalidate();
    }
    else
    {
        capacity_ = 1;
    }
}

bool Buffer::invalidated() const noexcept
{
    return invalidated_;
}

void Buffer::reserveSpaceFor(size_t bytesNum)
{
    if (invalidated())
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

size_t Buffer::size() const noexcept
{
    return size_;
}
const std::byte *Buffer::data() const noexcept
{
    return rawMemory_.get();
}

void Buffer::invalidate()
{
    invalidated_ = true;
    rawMemory_.reset();
    size_     = 0;
    capacity_ = 0;
}

void Buffer::addMemoryChunk(const std::byte *start, size_t size)
{
    reserveSpaceFor(size);

    // since reserveSpaceFor might invalidate the Buffer,
    // invalidated() check should happen here, but not at
    // the beginning of this method
    if (invalidated())
        return;

    std::memcpy(rawMemory_.get() + size_, start, size);
    size_ += size;
}

}    // namespace protocol
