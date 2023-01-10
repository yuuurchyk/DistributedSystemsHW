#pragma once

#include <atomic>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <tbb/concurrent_unordered_map.h>

#include "utils/copymove.h"

/**
 * @note all methods are thread safe
 */
class MasterStorage
{
    DISABLE_COPY_MOVE(MasterStorage)
public:
    MasterStorage()  = default;
    ~MasterStorage() = default;

    /**
     * @brief adds message into storage
     * @return size_t - id of added message (always increased)
     */
    size_t addMessage(std::string);

    /**
     * @brief get the contiguous chunk of messages starting from @p startId
     */
    std::vector<std::string_view> getContiguousChunk(size_t startId = 0) const;

private:
    std::atomic<size_t>                                nextMessageId_;
    tbb::concurrent_unordered_map<size_t, std::string> messages_;
};
