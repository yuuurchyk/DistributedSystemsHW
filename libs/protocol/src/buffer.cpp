#include "protocol/buffer.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace protocol
{

Buffer::Buffer() : Buffer::Buffer{ /* capacity */ 1 } {}

Buffer::Buffer(size_t capacity)
{
    rawMemory_.reset(new (std::nothrow) std::byte[capacity]);
    if (rawMemory_.get() == nullptr)
    {
        invalidate();
    }
    else
    {
        capacity_ = capacity;
    }
}

Buffer::Buffer(const Buffer &rhs)
{
    if (rhs.invalidated())
        invalidate();
    else
    {
        rawMemory_.reset(new std::byte[rhs.capacity()]);
        // TODO: handle the case of reset throwing exception
        size_     = rhs.size();
        capacity_ = rhs.capacity();
        std::memcpy(rawMemory_.get(), rhs.rawMemory_.get(), size());
    }
}

Buffer &Buffer::operator=(const Buffer &rhs)
{
    rawMemory_.reset(new std::byte[rhs.capacity()]);
    size_     = rhs.size();
    capacity_ = rhs.capacity();
    std::memcpy(rawMemory_.get(), rhs.rawMemory_.get(), size());
    return *this;
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

size_t Buffer::capacity() const noexcept
{
    return capacity_;
}

std::byte *Buffer::data() noexcept
{
    return rawMemory_.get();
}

const std::byte *Buffer::data() const noexcept
{
    return rawMemory_.get();
}

void Buffer::setSize(size_t newSize)
{
    if (invalidated())
        return;
    newSize = std::min(newSize, capacity());
    size_   = newSize;
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

BufferView::BufferView(const std::byte *data, size_t size) : data_{ data }, size_{ size }
{
}

const std::byte *BufferView::data() const noexcept
{
    return data_;
}
size_t BufferView::size() const noexcept
{
    return size_;
}

}    // namespace protocol
