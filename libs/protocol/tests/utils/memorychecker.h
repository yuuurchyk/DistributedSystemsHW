#pragma once

#include <cstddef>
#include <stdexcept>
#include <type_traits>

class MemoryChecker
{
public:
    MemoryChecker(const std::byte *ptr) : ptr{ ptr }
    {
        if (ptr == nullptr)
            throw std::runtime_error{ "MemoryChecker cannot work with nullptr memory" };
    }

    template <typename T>
    const T &checkAndAdvance()
    {
        static_assert(std::is_same_v<T, std::decay_t<T>>, "wrong template parameter");
        auto data = reinterpret_cast<const T *>(ptr);
        ptr += sizeof(T);
        return *data;
    }

private:
    const std::byte *ptr;
};
