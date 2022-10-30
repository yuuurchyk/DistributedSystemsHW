#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "utils/copymove.h"

class IOContextPool
{
    DISABLE_COPY_MOVE(IOContextPool);

public:
    // should be positive
    IOContextPool(size_t size);
    ~IOContextPool();

    // not thead safe
    void runInSeparateThreads(bool forever = true);
    void stop();

    // thread safe, no locks
    size_t                   size() const noexcept;
    boost::asio::io_context &getNext();
    boost::asio::io_context &getAtIndex(size_t index);

private:
    const size_t        size_;
    std::atomic<size_t> nextCounter_{};

    const std::vector<std::unique_ptr<boost::asio::io_context>> contexts_;

    using work_guard_t =
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    std::vector<std::thread>  workerThreads_;
    std::vector<work_guard_t> workGuards_;
};
