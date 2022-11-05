#pragma once

#include <string>
#include <string_view>

#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/channel_feature.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/type_index.hpp>

#include "logger/detail/numidfeature.hpp"
#include "logger/detail/severity.h"

namespace logger::detail
{
class EntityLogger
    : public boost::log::sources::basic_composite_logger<
          char,
          EntityLogger,
          boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>,
          boost::log::sources::features<boost::log::sources::severity<detail::Severity>,
                                        boost::log::sources::channel<std::string_view>>>
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS_TEMPLATE(EntityLogger)
};

class NumIdEntityLogger
    : public boost::log::sources::basic_composite_logger<
          char,
          NumIdEntityLogger,
          boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>,
          boost::log::sources::features<boost::log::sources::severity<detail::Severity>,
                                        boost::log::sources::channel<std::string_view>,
                                        detail::NumId>>
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS_TEMPLATE(NumIdEntityLogger)
};

template <typename Entity_t>
class EntityBase
{
public:
    // clang-format off
    static inline const std::string kEntityNameStr_{ boost::typeindex::type_id<Entity_t>().pretty_name() };
    static inline const std::string_view kEntityName{ kEntityNameStr_ };
    // clang-format on
};

}    // namespace logger::detail
