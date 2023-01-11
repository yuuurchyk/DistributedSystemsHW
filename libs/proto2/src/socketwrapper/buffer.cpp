#include "socketwrapper/buffer.h"

namespace Proto2
{
size_t Buffer::size() const noexcept
{
    return size_;
}

std::byte *Buffer::get()
{
    return memory_.get();
}

bool Buffer::resize(size_t bytes)
{
    if (bytes == size())
        return true;

    memory_ = {};
    size_   = {};

    memory_.reset(new (std::nothrow) std::byte[bytes]);

    if (memory_ == nullptr)
    {
        return false;
    }
    else
    {
        size_ = bytes;
        return true;
    }
}

}    // namespace Proto2
