#include "formatter.hpp"

#include <iomanip>
#include <string_view>
#include <type_traits>

#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions.hpp>
#include <boost/scope_exit.hpp>

#include "logger/detail/attributes.h"
#include "logger/detail/severity.h"
#include "logger/identity.hpp"

namespace logger
{
void formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &strm)
{
    using namespace detail::attributes;

    if (const auto programNameIt = rec[kProgramNameAttr].extract<program_name_t>())
    {
        strm << "[" << *programNameIt;
        BOOST_SCOPE_EXIT(&strm)
        {
            strm << "]";
        }
        BOOST_SCOPE_EXIT_END

        if (const auto threadIdIt = rec[kThreadIdAttr].extract<thread_id_t>())
        {
            strm << "," << std::hex << ((*threadIdIt).native_id() & (0xfffff))
                 << std::dec;
        }
    }

    if (const auto entityNameIt = rec[kEntityNameAttr].extract<entity_name_t>())
    {
        strm << " [" << *entityNameIt;
        BOOST_SCOPE_EXIT(&strm)
        {
            strm << "]";
        }
        BOOST_SCOPE_EXIT_END

        if (const auto loggerIdIt = rec[kIdAttr].extract<logger_id_t>())
            strm << ",id=" << std::setw(3) << *loggerIdIt;
    }

    {
        const auto fileNameIt   = rec[kFilenameAttr].extract<file_name_t>();
        const auto lineNumberIt = rec[kLineNumberAttr].extract<line_number_t>();

        if (fileNameIt && lineNumberIt)
            strm << " [" << *fileNameIt << ":" << *lineNumberIt << "]";
    }

    if (auto severityIt = rec[kSeverityAttr].extract<::logger::detail::Severity>())
        strm << " [" << *severityIt << "]";

    strm << " : " << rec[boost::log::expressions::smessage];
}

}    // namespace logger
