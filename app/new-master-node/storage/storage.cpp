#include "storage/storage.h"

#include <utility>

size_t Storage::addMessage(std::string message)
{
    const auto id = nextMessageId_.fetch_add(1, std::memory_order_relaxed);
    messages_.insert({ id, std::move(message) });
    return id;
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
