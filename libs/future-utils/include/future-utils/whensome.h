#pragma once

#include <vector>

#include <boost/thread.hpp>

namespace futureUtils
{
/**
 * @brief the resulting future will be ready when there is at least @p minCount available
 * results
 *
 * @note order of @p futures is preserved
 * @todo add executor
 */
template <typename T>
boost::future<std::vector<boost::future<T>>>
    whensome(std::vector<boost::future<T>> futures, size_t minCount);

}    // namespace futureUtils

#include "detail/whensome_impl.hpp"
