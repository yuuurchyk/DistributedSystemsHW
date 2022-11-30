#pragma once

#include "future-utils/whensome.h"

#include <stdexcept>

#include "utils/copymove.h"

namespace futureUtils
{
namespace detail
{
    template <typename T>
    struct WhenSomeContext : std::enable_shared_from_this<WhenSomeContext<T>>
    {
        DISABLE_COPY_MOVE(WhenSomeContext);

        WhenSomeContext(std::vector<boost::future<T>> futures, size_t minCount)
            : minCount{ minCount }, futures{ std::move(futures) }
        {
        }

        const size_t                  minCount;
        std::vector<boost::future<T>> futures;

        // pending futures go into pending vector, with original indexes
        // remembered in futuresIndexes
        std::vector<boost::future<T>> pending;
        std::vector<size_t>           pendingIndexes;

        struct Status
        {
            size_t pendingCnt{};
            size_t readyValueCnt{};
            size_t readyExceptionCnt{};
        };

        Status status() const
        {
            auto status = Status{};

            for (auto &future : futures)
            {
                if (!future.is_ready())
                    ++status.pendingCnt;
                else if (future.has_value())
                    ++status.readyValueCnt;
                else
                    ++status.readyExceptionCnt;
            }

            return status;
        }

        void constructPending()
        {
            pending.clear();
            pendingIndexes.clear();

            for (auto i = size_t{}; i < futures.size(); ++i)
            {
                if (futures[i].is_ready())
                    continue;

                pending.push_back(std::move(futures[i]));
                pendingIndexes.push_back(i);
            }
        }
        void reconstructFromPending()
        {
            if (pending.size() != pendingIndexes.size())
                throw std::logic_error{
                    "WhenSomeContext::reconstructFromPending internal error"
                };

            for (auto pendingI = size_t{}; pendingI < pending.size(); ++pendingI)
            {
                const auto futuresI = pendingIndexes[pendingI];

                if (futuresI >= futures.size())
                    throw std::logic_error{
                        "WhenSomeContext::reconstructFromPending internal error"
                    };

                futures[futuresI] = std::move(pending[pendingI]);
            }

            pending.clear();
            pendingIndexes.clear();
        }

        boost::future<std::vector<boost::future<T>>> run()
        {
            if (const auto status = this->status();
                status.readyValueCnt >= minCount || status.pendingCnt == 0)
            {
                return boost::make_ready_future(std::move(futures));
            }
            else
            {
                constructPending();
                return boost::when_any(pending.begin(), pending.end())
                    .then([this, self = this->shared_from_this()](
                              boost::future<std::vector<boost::future<T>>> pendingFuture)
                          { return callback(std::move(pendingFuture)); })
                    .unwrap();
            }
        }

        boost::future<std::vector<boost::future<T>>>
            callback(boost::future<std::vector<boost::future<T>>> pendingFuture)
        {
            if (pendingFuture.has_exception())
                pendingFuture.get();
            if (!pendingFuture.has_value())
                throw std::logic_error{ "WhenSomeContext::callback internal error" };

            pending = std::move(pendingFuture.get());
            reconstructFromPending();

            if (const auto status = this->status();
                status.readyValueCnt >= minCount || status.pendingCnt == 0)
            {
                return boost::make_ready_future(std::move(futures));
            }
            else
            {
                constructPending();

                return boost::when_any(pending.begin(), pending.end())
                    .then([this, self = this->shared_from_this()](
                              boost::future<std::vector<boost::future<T>>> pendingFuture)
                          { return callback(std::move(pendingFuture)); })
                    .unwrap();
            }
        }
    };
}    // namespace detail

template <typename T>
boost::future<std::vector<boost::future<T>>>
    whensome(std::vector<boost::future<T>> futures, size_t minCount)
{
    if (minCount == 0)
        return boost::make_ready_future(std::move(futures));

    auto context =
        std::make_shared<detail::WhenSomeContext<T>>(std::move(futures), minCount);

    return context->run();
}

}    // namespace futureUtils
