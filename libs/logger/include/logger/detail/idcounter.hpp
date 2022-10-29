#pragma once

#include <atomic>
#include <cstddef>

#include "utils/copymove.h"

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
    IdCounter()  = delete;
    ~IdCounter() = delete;
    DISABLE_COPY_MOVE(IdCounter);

public:
    static logger_id_t getNext()
    {
        return counter_.fetch_add(1, std::memory_order_relaxed);
    }

private:
    static inline std::atomic<logger_id_t> counter_{};
};

}    // namespace logger::detail
