#pragma once

#include <string_view>

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include "logger/detail/extractbasename.hpp"
#include "logger/detail/severity.h"

namespace logger::detail
{
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
    GlobalLogger,
    boost::log::sources::severity_logger_mt<::logger::detail::Severity>);

}    // namespace logger::detail

#define _LOGIMPL(severity)                                                \
    BOOST_LOG_SEV(::logger::detail::GlobalLogger::get(),                  \
                  ::logger::detail::Severity::severity)                   \
        << ::boost::log::add_value(                                       \
               ::logger::detail::attributes::kFilenameAttr,               \
               ::logger::detail::attributes::file_name_t{ __FILENAME__ }) \
        << ::boost::log::add_value(                                       \
               ::logger::detail::attributes::kLineNumberAttr,             \
               ::logger::detail::attributes::line_number_t{ __LINE__ })

#define LOGI _LOGIMPL(Info)
#define LOGW _LOGIMPL(Warning)
#define LOGE _LOGIMPL(Error)
