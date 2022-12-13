#include "net-utils/iocontextpool.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

using namespace boost::asio;

namespace NetUtils
{
std::shared_ptr<IOContextPool> IOContextPool::create(size_t size)
{
    return std::shared_ptr<IOContextPool>{ new IOContextPool{ size } };
}

IOContextPool::IOContextPool(size_t size)
    : size_{ std::max<size_t>(size, 1) },
      contexts_{ [size]()
                 {
                     const auto n = std::max<size_t>(size, 1);

                     auto res = std::vector<std::unique_ptr<io_context>>{};

                     res.reserve(n);

                     for (auto i = size_t{}; i < n; ++i)
                         res.push_back(std::make_unique<io_context>());

                     return res;
                 }() }
{
    if (size == 0)
        EN_LOGW << "replacing size 0 with 1";
    load_.resize(this->size(), 0);
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

auto IOContextPool::getLeastLoaded() -> std::unique_ptr<LoadGuard>
{
    std::unique_lock<std::mutex> lock{ loadMutex_ };

    auto smallestLoad      = std::numeric_limits<size_t>::max();
    auto smallestLoadIndex = size_t{};

    for (auto i = size_t{}; i < size(); ++i)
    {
        if (smallestLoad <= load_[i])
            continue;

        smallestLoad      = load_[i];
        smallestLoadIndex = i;
    }

    ++load_[smallestLoadIndex];

    lock.unlock();

    return std::unique_ptr<LoadGuard>{ new LoadGuard{ weak_from_this(), *contexts_[smallestLoadIndex] } };
}

void IOContextPool::decLoad(boost::asio::io_context &context)
{
    std::unique_lock<std::mutex> lock{ loadMutex_ };

    for (auto i = size_t{}; i < size(); ++i)
    {
        if (contexts_[i].get() != &context)
            continue;
        if (load_[i] > 0)
            --load_[i];
    }
}

IOContextPool::LoadGuard::LoadGuard(std::weak_ptr<IOContextPool> weakPool, boost::asio::io_context &ioContext)
    : ioContext_{ ioContext }, weakPool_{ std::move(weakPool) }
{
}

IOContextPool::LoadGuard::~LoadGuard()
{
    auto pool = weakPool_.lock();

    if (pool == nullptr)
        return;

    pool->decLoad(ioContext_);
}

namespace
{
    void ioContextRunner(io_context &context)
    {
        context.run();
    }
}    // namespace

void IOContextPool::runInSeparateThreads(bool forever)
{
    if (!workerThreads_.empty() || !workGuards_.empty())
        throw std::runtime_error{ "IOContextPool::runInSeparateThreads: wrong usage" };

    if (forever)
    {
        workGuards_.reserve(size());
        for (auto &context : contexts_)
            workGuards_.emplace_back(context->get_executor());
    }

    workerThreads_.reserve(size());
    for (auto &context : contexts_)
        workerThreads_.emplace_back(ioContextRunner, std::ref(*context));
}

void IOContextPool::stop()
{
    for (auto &context : contexts_)
        context->stop();

    for (auto &thread : workerThreads_)
        thread.join();

    workerThreads_.clear();
    workGuards_.clear();
}

IOContextPool::~IOContextPool()
{
    if (!workerThreads_.empty())
        stop();
}

}    // namespace NetUtils
