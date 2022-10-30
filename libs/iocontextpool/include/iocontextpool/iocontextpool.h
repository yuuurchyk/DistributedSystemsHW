#pragma once

#include <memory>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>

#include "utils/copymove.h"

class IOContextPool
{
    DISABLE_COPY_MOVE(IOContextPool);

public:
    // should be positive
    IOContextPool(size_t size);

    // not thead safe
    void runInSeparateThreads(bool forever = true);
    void stop();

    // thread safe, no locks
    size_t                   size() const noexcept;
    boost::asio::io_context &getNext();
    boost::asio::io_context &getAtIndex(size_t index);

    // thread safe, require locks
    void                     incLoad(const boost::asio::io_context &);
    void                     decLoad(const boost::asio::io_context &);
    boost::asio::io_context &getLeastLoaded();

private:
    size_t              size_;
    std::atomic<size_t> nextCounter_{};

    std::vector<std::unique_ptr<boost::asio::io_context>>       contexts_;
    std::unordered_map<const boost::asio::io_context *, size_t> contextToIndex_;

    std::shared_mutex      loadLock_;
    std::vector<long long> load_;

    using work_guard_t =
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    std::vector<std::thread>  workerThreads_;
    std::vector<work_guard_t> workGuards_;
};
