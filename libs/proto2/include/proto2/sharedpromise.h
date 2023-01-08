#pragma once

#include <memory>

#include <boost/thread.hpp>

namespace Proto2
{
template <typename T>
using SharedPromise = std::shared_ptr<boost::promise<T>>;

}    // namespace Proto2
