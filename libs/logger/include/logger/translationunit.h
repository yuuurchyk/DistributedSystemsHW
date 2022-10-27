#pragma once

#include <string_view>

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include "logger/severity.h"

namespace logger
{
class TranslationUnitLogger : public boost::log::sources::severity_logger_mt<Severity>
{
public:
    TranslationUnitLogger(std::string_view moduleName);
};

}    // namespace logger

#define DEFINE_TU_LOGGER(moduleName)                             \
    namespace                                                    \
    {                                                            \
        ::logger::TranslationUnitLogger _tuLogger{ moduleName }; \
    }

#define TU_LOGI BOOST_LOG_SEV(::_tuLogger, ::logger::Severity::Info)
#define TU_LOGW BOOST_LOG_SEV(::_tuLogger, ::logger::Severity::Warning)
#define TU_LOGE BOOST_LOG_SEV(::_tuLogger, ::logger::Severity::Error)
