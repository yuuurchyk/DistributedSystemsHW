#pragma once

#include <string>
#include <string_view>

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

#include "logger/detail/extractbasename.hpp"
#include "logger/detail/severity.h"
#include "logger/entity.hpp"

namespace logger
{
// NOTE: these functions should be called only from main thread at start and end
// of the program
void setup(std::string programName);
void teardown();

namespace detail
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
        GlobalLogger,
        boost::log::sources::severity_logger_mt<::logger::detail::Severity>);
}

}    // namespace logger

namespace logger::detail
{

}    // namespace logger::detail

#define _LOGIMPL(severity)                                                            \
    BOOST_LOG_SEV(::logger::detail::GlobalLogger::get(),                              \
                  ::logger::detail::Severity::severity)                               \
        << ::boost::log::add_value(                                                   \
               ::logger::detail::attributes::kFilename,                               \
               ::logger::detail::attributes::file_name_t{                             \
                   ::logger::detail::extractBaseName(std::string_view{ __FILE__ }) }) \
        << ::boost::log::add_value(                                                   \
               ::logger::detail::attributes::kLineNumber,                             \
               ::logger::detail::attributes::line_number_t{ __LINE__ })

#define LOGI _LOGIMPL(Info)
#define LOGW _LOGIMPL(Warning)
#define LOGE _LOGIMPL(Error)
