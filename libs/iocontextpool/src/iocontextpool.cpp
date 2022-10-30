#include "iocontextpool/iocontextpool.h"

#include <functional>
#include <stdexcept>
#include <string>

using namespace boost::asio;

IOContextPool::IOContextPool(size_t size)
    : size_{ size },
      contexts_{ [size]()
                 {
                     auto res = std::vector<std::unique_ptr<io_context>>{};

                     res.reserve(size);

                     for (auto i = size_t{}; i < size; ++i)
                         res.push_back(std::make_unique<io_context>());

                     return res;
                 }() }
{
    if (size == 0)
        throw std::runtime_error{ "IOContextPool should have positive size" };
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
    {
        throw std::out_of_range{ "IOContextPool::getAtIndex(" + std::to_string(index) +
                                 ")" };
    }

    return *contexts_[index];
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
