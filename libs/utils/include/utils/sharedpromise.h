#pragma once

#include <memory>
#include <utility>

#include <boost/thread.hpp>

namespace Utils
{
template <typename T>
using SharedPromise = std::shared_ptr<boost::promise<T>>;

template <typename T>
Utils::SharedPromise<T> makeSharedPromise(boost::promise<T> promise)
{
    return std::make_shared<boost::promise<T>>(std::move(promise));
}

}    // namespace Utils
