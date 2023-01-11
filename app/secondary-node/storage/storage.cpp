#include "storage/storage.h"

#include <utility>

void Storage::addMessage(size_t id, std::string message)
{
    const auto res = messages_.insert({ id, std::move(message) });
    if (!res.second)
    {
        EN_LOGW << "failed to insert message with id " << id << ", already present";
    }
}

void Storage::addMessages(size_t startId, std::vector<std::string> messages)
{
    for (auto i = size_t{}; i < messages.size(); ++i)
        addMessage(startId + i, std::move(messages[i]));
}

size_t Storage::getFirstGap() const
{
    auto res = size_t{};

    while (true)
    {
        const auto it = messages_.find(res);
        if (it == messages_.end())
            break;
        else
            ++res;
    }

    return res;
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
