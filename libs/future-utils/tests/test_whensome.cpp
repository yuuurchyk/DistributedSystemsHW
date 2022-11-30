#include "future-utils/whensome.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <boost/thread.hpp>
#include <gtest/gtest.h>

#include "logger/logger.h"
#include "utils/copymove.h"

namespace
{

class Context
{
    DISABLE_COPY_MOVE(Context);

public:
    Context()
    {
        auto p1 = boost::promise<int>{};
        auto p2 = boost::promise<int>{};
        futures.push_back(p1.get_future());
        futures.push_back(p2.get_future());

        threads.emplace_back(
            [promise = std::move(p1)]() mutable
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
                promise.set_value(1);
            });
        threads.emplace_back(
            [promise = std::move(p2)]() mutable
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 200 });
                promise.set_value(2);
            });
    }
    ~Context()
    {
        for (auto &thread : threads)
            thread.join();
    }

    std::vector<boost::future<int>> getFutures() { return std::move(futures); }

private:
    std::vector<boost::future<int>> futures;
    std::vector<std::thread>        threads;
};

TEST(WhenSome, MinCount0)
{
    auto context = std::make_shared<Context>();

    auto checksPerformed = std::atomic<bool>{};
    futureUtils::whensome(context->getFutures(), 0)
        .then(
            [&](boost::future<std::vector<boost::future<int>>> resultsFuture)
            {
                ASSERT_TRUE(resultsFuture.has_value());

                auto results = std::vector<boost::future<int>>{};
                ASSERT_NO_THROW(results = resultsFuture.get());

                for (auto &future : results)
                    ASSERT_FALSE(future.is_ready());

                checksPerformed.store(true, std::memory_order_relaxed);
            });

    context = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    ASSERT_TRUE(checksPerformed.load(std::memory_order_relaxed));
}

TEST(WhenSome, MinCount1)
{
    auto context = std::make_shared<Context>();

    auto checksPerformed = std::atomic<bool>{};
    futureUtils::whensome(context->getFutures(), 1)
        .then(
            [&](boost::future<std::vector<boost::future<int>>> resultsFuture)
            {
                ASSERT_TRUE(resultsFuture.has_value());

                auto results = std::vector<boost::future<int>>{};
                ASSERT_NO_THROW(results = resultsFuture.get());

                ASSERT_EQ(results.size(), 2);

                ASSERT_TRUE(results[0].is_ready());
                ASSERT_FALSE(results[1].is_ready());
                ASSERT_EQ(results[0].get(), 1);

                checksPerformed.store(true, std::memory_order_relaxed);
            });

    context = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    ASSERT_TRUE(checksPerformed.load(std::memory_order_relaxed));
}

TEST(WhenSome, MinCount2)
{
    auto context = std::make_shared<Context>();

    auto checksPerformed = std::atomic<bool>{};
    futureUtils::whensome(context->getFutures(), 2)
        .then(
            [&](boost::future<std::vector<boost::future<int>>> resultsFuture)
            {
                ASSERT_TRUE(resultsFuture.has_value());

                auto results = std::vector<boost::future<int>>{};
                ASSERT_NO_THROW(results = resultsFuture.get());

                ASSERT_EQ(results.size(), 2);

                ASSERT_TRUE(results[0].is_ready());
                ASSERT_TRUE(results[1].is_ready());
                ASSERT_EQ(results[0].get(), 1);
                ASSERT_EQ(results[1].get(), 2);

                checksPerformed.store(true, std::memory_order_relaxed);
            });

    context = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    ASSERT_TRUE(checksPerformed.load(std::memory_order_relaxed));
}

TEST(WhenSome, MinCount3)
{
    auto context = std::make_shared<Context>();

    auto checksPerformed = std::atomic<bool>{};
    futureUtils::whensome(context->getFutures(), 3)
        .then(
            [&](boost::future<std::vector<boost::future<int>>> resultsFuture)
            {
                ASSERT_TRUE(resultsFuture.has_value());

                auto results = std::vector<boost::future<int>>{};
                ASSERT_NO_THROW(results = resultsFuture.get());

                ASSERT_EQ(results.size(), 2);

                ASSERT_TRUE(results[0].is_ready());
                ASSERT_TRUE(results[1].is_ready());
                ASSERT_EQ(results[0].get(), 1);
                ASSERT_EQ(results[1].get(), 2);

                checksPerformed.store(true, std::memory_order_relaxed);
            });

    context = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    ASSERT_TRUE(checksPerformed.load(std::memory_order_relaxed));
}

TEST(WhenSome, FuturesOrder)
{
    auto futures = std::vector<boost::future<int>>{};
    auto threads = std::vector<std::thread>{};

    for (auto delay : { 100, 50, 25, 75 })
    {
        auto promise = boost::promise<int>{};
        futures.push_back(promise.get_future());
        threads.emplace_back(
            [delay, promise = std::move(promise)]() mutable
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{ delay });
                promise.set_value(delay);
            });
    }

    auto checksPerformed = std::atomic<bool>{};
    futureUtils::whensome(std::move(futures), 2)
        .then(
            [&](boost::future<std::vector<boost::future<int>>> resultsFuture)
            {
                ASSERT_TRUE(resultsFuture.has_value());

                auto results = std::vector<boost::future<int>>{};
                ASSERT_NO_THROW(results = resultsFuture.get());

                ASSERT_EQ(results.size(), 4);

                ASSERT_FALSE(results[0].is_ready());
                ASSERT_TRUE(results[1].is_ready());
                ASSERT_TRUE(results[2].is_ready());
                ASSERT_FALSE(results[3].is_ready());

                ASSERT_EQ(results[1].get(), 50);
                ASSERT_EQ(results[2].get(), 25);

                checksPerformed.store(true, std::memory_order_relaxed);
            });

    for (auto &thread : threads)
        thread.join();
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    ASSERT_TRUE(checksPerformed.load(std::memory_order_relaxed));
}

}    // namespace
