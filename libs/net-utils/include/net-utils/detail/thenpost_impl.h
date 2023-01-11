#pragma once

#include <type_traits>
#include <utility>

#include "net-utils/thenpost.h"

namespace NetUtils
{

namespace detail
{

    template <typename T>
    struct IsBoostFuture : std::false_type
    {
    };

    template <typename T>
    struct IsBoostFuture<boost::future<T>> : std::true_type
    {
    };

    template <typename T>
    static constexpr bool is_boost_future_v = IsBoostFuture<T>::value;

}    // namespace detail

template <typename Future, typename Functor>
void thenPost(Future &&future, boost::asio::io_context &ioContext, Functor &&callback)
{
    static_assert(detail::is_boost_future_v<std::decay_t<Future>>, "Future type should be a boost future");
    using value_type = typename std::decay_t<Future>::value_type;

    static_assert(std::is_invocable_v<std::decay_t<Functor>, boost::future<value_type>>, "Wrong callable signature");

    future.then(
        [&ioContext, callback = std::move(callback)](boost::future<value_type> future) mutable
        {
            boost::asio::post(
                ioContext,
                [future = std::move(future), callback = std::move(callback)]() mutable
                { callback(std::move(future)); });
        });
}

}    // namespace NetUtils
