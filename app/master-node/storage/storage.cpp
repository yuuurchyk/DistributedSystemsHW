#include "storage/storage.h"

#include <utility>

std::pair<size_t, std::string_view> Storage::addMessage(std::string message)
{
    const auto id  = nextMessageId_.fetch_add(1, std::memory_order_relaxed);
    auto       res = messages_.insert({ id, std::move(message) });

    return std::make_pair(id, std::string_view{ res.first->second });
}

std::vector<std::string_view> Storage::getContiguousChunk(size_t startId) const
{
    auto res = std::vector<std::string_view>{};

    while (true)
    {
        const auto it = messages_.find(startId);
        if (it == messages_.end())
            break;

        res.push_back(it->second);
        ++startId;
    }

    return res;
}
