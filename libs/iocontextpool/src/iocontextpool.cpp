#include "iocontextpool/iocontextpool.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

using namespace boost::asio;

IOContextPool::IOContextPool(size_t size) : size_{ size }
{
    if (size == 0)
        throw std::runtime_error{ "IOContextPool should have positive size" };

    contexts_.reserve(size);
    contextToIndex_.reserve(size);
    load_.reserve(size);

    for (auto i = size_t{}; i < size; ++i)
    {
        contexts_.push_back(std::make_unique<io_context>());
        contextToIndex_[contexts_.back().get()] = i;
        load_.push_back(0);
    }
}

size_t IOContextPool::size() const noexcept
{
    return size_;
}

io_context &IOContextPool::getNext()
{
    const auto val = nextCounter_.fetch_add(1, std::memory_order_relaxed);
    return *contexts_[val % size()];
}

io_context &IOContextPool::getAtIndex(size_t index)
{
    if (index >= size())
        throw std::out_of_range{ "IOContextPool::getAtIndex(" + std::to_string(index) +
                                 ")" };

    return *contexts_[index];
}

void IOContextPool::incLoad(const boost::asio::io_context &context)
{
    const auto it = contextToIndex_.find(&context);
    if (it == contextToIndex_.end())
        return;

    const auto index = it->second;

    std::unique_lock<std::shared_mutex> lck{ loadLock_ };
    ++load_[index];
}

void IOContextPool::decLoad(const boost::asio::io_context &context)
{
    const auto it = contextToIndex_.find(&context);
    if (it == contextToIndex_.end())
        return;

    const auto index = it->second;

    std::unique_lock<std::shared_mutex> lck{ loadLock_ };
    --load_[index];
}

io_context &IOContextPool::getLeastLoaded()
{
    auto targetIndex = std::numeric_limits<size_t>::max();
    auto minLoad     = std::numeric_limits<long long>::max();

    {
        std::shared_lock<std::shared_mutex> lck{ loadLock_ };

        for (auto i = size_t{}; i < size(); ++i)
        {
            if (load_[i] <= minLoad)
            {
                minLoad     = load_[i];
                targetIndex = i;
            }
        }
    }

    if (targetIndex >= size())
        throw std::logic_error{ "IOContextPool::getLeastLoaded(): internal error" };

    return *contexts_[targetIndex];
}
