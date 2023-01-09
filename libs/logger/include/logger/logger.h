#pragma once

#include <string>
#include <string_view>

#include <boost/log/detail/light_rw_mutex.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>

#include "logger/detail/codelocation.hpp"
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
    class GlobalLogger
        : public boost::log::sources::basic_composite_logger<
              char,
              GlobalLogger,
              boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>,
              boost::log::sources::features<
                  boost::log::sources::severity<detail::Severity>,
                  detail::KeyValue<CodeFilenameKeywordGetter,
                                   CodeFilenameAttrNameGetter,
                                   attributes::code_file_name_t>,
                  detail::KeyValue<CodeLineNumberKeywordGetter,
                                   CodeLineNumberAttrNameGetter,
                                   attributes::code_line_number_t>>>
    {
        BOOST_LOG_FORWARD_LOGGER_MEMBERS_TEMPLATE(GlobalLogger)
    };

    BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(m_globalLogger, GlobalLogger);
}    // namespace detail

}    // namespace logger

#define _LOGIMPL(sev)                                                             \
    BOOST_LOG_WITH_PARAMS(                                                        \
        ::logger::detail::m_globalLogger::get(),                                  \
        (boost::log::keywords::severity = ::logger::detail::Severity::sev)(       \
            ::logger::detail::keywords::CodeFilename =                            \
                ::logger::detail::extractBaseName(std::string_view{ __FILE__ }))( \
            ::logger::detail::keywords::CodeLineNumber =                          \
                ::logger::detail::attributes::code_line_number_t{ __LINE__ }))

#define LOGD _LOGIMPL(Debug)
#define LOGI _LOGIMPL(Info)
#define LOGW _LOGIMPL(Warning)
#define LOGE _LOGIMPL(Error)
