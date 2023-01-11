#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <tbb/concurrent_unordered_map.h>

#include "logger/logger.h"
#include "utils/copymove.h"

/**
 * @note all methods are thread safe
 */
class Storage : private logger::Entity<Storage>
{
    DISABLE_COPY_MOVE(Storage)
public:
    Storage()  = default;
    ~Storage() = default;

    void addMessage(size_t id, std::string message);
    void addMessages(size_t startId, std::vector<std::string> messages);

    /**
     * @brief get the first id, starting from 0, which is not present in messages
     * @note thread safe
     */
    size_t getFirstGap() const;

    /**
     * @brief get the contiguous chunk of messages starting from @p startId
     */
    std::vector<std::string_view> getContiguousChunk(size_t startId = 0) const;

private:
    tbb::concurrent_unordered_map<size_t, std::string> messages_;
};
