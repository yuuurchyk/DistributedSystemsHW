#pragma once

#include <cstdint>
#include <atomic>

namespace logger
{
// we assume that we won't have many objects
using num_id_t = uint16_t;

template <typename Entity_t>
class IdCounter
{
public:
    static num_id_t getNext() { return counter_.fetch_add(1, std::memory_order_relaxed); }

private:
    static inline std::atomic<num_id_t> counter_{};

    IdCounter()                             = delete;
    IdCounter(const IdCounter &)            = delete;
    IdCounter(IdCounter &&)                 = delete;
    IdCounter &operator=(const IdCounter &) = delete;
    IdCounter &operator=(IdCounter &&)      = delete;
    ~IdCounter()                            = delete;
};

}    // namespace logger
