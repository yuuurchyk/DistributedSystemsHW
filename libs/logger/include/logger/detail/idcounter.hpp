#pragma once

#include <atomic>
#include <cstddef>

namespace logger::detail
{
using logger_id_t = size_t;

/**
 * @brief
 *
 * @tparam Entity_t
 *
 * @note make sure that the class that derives from
 * IdCounter is final
 */
template <typename Entity_t>
class IdCounter
{
public:
    static logger_id_t getNext()
    {
        return counter_.fetch_add(1, std::memory_order_relaxed);
    }

private:
    static inline std::atomic<logger_id_t> counter_{};

    IdCounter()                             = delete;
    IdCounter(const IdCounter &)            = delete;
    IdCounter(IdCounter &&)                 = delete;
    IdCounter &operator=(const IdCounter &) = delete;
    IdCounter &operator=(IdCounter &&)      = delete;
    ~IdCounter()                            = delete;
};

}    // namespace logger::detail
