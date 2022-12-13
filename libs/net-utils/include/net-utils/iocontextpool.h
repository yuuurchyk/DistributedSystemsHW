#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

namespace NetUtils
{
class IOContextPool : public std::enable_shared_from_this<IOContextPool>, private logger::Entity<IOContextPool>
{
    DISABLE_COPY_MOVE(IOContextPool);

public:
    // should be positive
    [[nodiscard]] static std::shared_ptr<IOContextPool> create(size_t size);

    ~IOContextPool();

    // not thead safe
    void runInSeparateThreads(bool forever = true);
    void stop();

    // thread safe, no locks
    size_t                   size() const noexcept;
    boost::asio::io_context &getNext();

    /**
     * @brief once LoadGuard is destroyed, the load
     * of associated io_context is decreased
     */
    struct LoadGuard
    {
        DISABLE_COPY_MOVE(LoadGuard)

        boost::asio::io_context &ioContext_;

        ~LoadGuard();

    private:
        friend class IOContextPool;

        LoadGuard(std::weak_ptr<IOContextPool> pool, boost::asio::io_context &ioContext);

        std::weak_ptr<IOContextPool> weakPool_;
    };

    /**
     * @brief get least loaded io_context and increase its load
     * @note thread safe, with locking
     */
    std::unique_ptr<LoadGuard> getLeastLoaded();

private:
    IOContextPool(size_t size);

    void decLoad(boost::asio::io_context &);

    using work_guard_t = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    const size_t        size_;
    std::atomic<size_t> nextCounter_{};

    const std::vector<std::unique_ptr<boost::asio::io_context>> contexts_;

    std::mutex          loadMutex_;
    std::vector<size_t> load_;

    std::vector<std::thread>  workerThreads_;
    std::vector<work_guard_t> workGuards_;
};

}    // namespace NetUtils
