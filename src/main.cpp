#include <functional>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#define BOOST_THREAD_VERSION 4

#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

#include "logger/logger.h"

int main()
{
    logger::setup("sample");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    auto context = boost::asio::io_context{};
    auto work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>{
        context.get_executor()
    };

    auto p1 = boost::promise<int>{};
    auto p2 = boost::promise<int>{};

    auto f1 = p1.get_future();
    auto f2 = p2.get_future();

    auto t1 = boost::asio::deadline_timer{ context };
    t1.expires_from_now(boost::posix_time::milliseconds{ 2000 });
    auto t2 = boost::asio::deadline_timer{ context };
    t2.expires_from_now(boost::posix_time::milliseconds{ 5000 });

    t1.async_wait(
        [promise = std::move(p1)](const boost::system::error_code &ec) mutable
        {
            if (!ec)
                promise.set_value(1);
        });
    t2.async_wait(
        [promise = std::move(p2)](const boost::system::error_code &ec) mutable
        {
            if (!ec)
            {
                LOGI << "INSIDE t2";
                promise.set_value(2);
            }
        });

    std::function<void(boost::future<std::vector<boost::future<int>>>)> callback{};
    callback =
        [&callback](boost::future<std::vector<boost::future<int>>> combined) mutable
    {
        LOGI << "callback";

        if (!combined.has_value())
        {
            LOGE << "does not have value";
            return;
        }
        else
        {
            auto futures = combined.get();
            auto allGood = true;

            for (auto &item : futures)
            {
                if (!item.is_ready())
                {
                    LOGW << "NOT READY";
                    allGood = false;
                    break;
                }
                else
                {
                    LOGI << "READY";
                }
            }

            if (!allGood)
            {
                boost::when_any(futures.begin() + 1, futures.end())
                    .then([&callback](
                              boost::future<std::vector<boost::future<int>>> combined)
                          { callback(std::move(combined)); });
            }
            else
            {
                LOGI << "All good";
                for (auto &item : futures)
                    LOGI << item.get();
            }
        }
    };

    auto futures = std::vector<boost::future<int>>{};
    futures.push_back(std::move(f1));
    futures.push_back(std::move(f2));
    boost::when_any(futures.begin(), futures.end())
        .then([&callback](boost::future<std::vector<boost::future<int>>> combined)
              { callback(std::move(combined)); });

    context.run();

    return 0;
}
