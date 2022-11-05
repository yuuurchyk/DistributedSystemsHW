#include "formatter.hpp"

#include <iomanip>
#include <string_view>
#include <type_traits>

#include <iostream>

#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions.hpp>
#include <boost/scope_exit.hpp>

#include "logger/detail/attributes.h"
#include "logger/detail/severity.h"

namespace logger
{
void formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &strm)
{
    using namespace detail::attributes;

    if (const auto programNameIt = rec[kProgramName].extract<program_name_t>())
    {
        strm << "[" << *programNameIt;
        BOOST_SCOPE_EXIT(&strm)
        {
            strm << "]";
        }
        BOOST_SCOPE_EXIT_END

        if (const auto threadIdIt = rec[kThreadId].extract<thread_id_t>())
        {
            strm << "," << std::hex << std::setw(5)
                 << ((*threadIdIt).native_id() & (0xfffff)) << std::dec;
        }
    }

    if (const auto fileNameIt = rec[kCodeFilename].extract<code_file_name_t>())
    {
        strm << " [" << *fileNameIt;

        BOOST_SCOPE_EXIT(&strm)
        {
            strm << "]";
        }
        BOOST_SCOPE_EXIT_END

        if (const auto lineNumberIt = rec[kCodeLineNumber].extract<code_line_number_t>())
        {
            strm << ":" << std::setw(4) << *lineNumberIt;
        }
        else
        {
            strm << std::setw(5) << "";
        }
    }

    if (const auto channelIt = rec[kChannel].extract<channel_t>())
    {
        strm << " [" << *channelIt;
        BOOST_SCOPE_EXIT(&strm)
        {
            strm << "]";
        }
        BOOST_SCOPE_EXIT_END

        if (const auto numIdIt = rec[kNumId].extract<num_id_t>())
            strm << ",id=" << std::setw(5) << *numIdIt;
        if (const auto stringIdIt = rec[kStringId].extract<string_id_t>())
            strm << ",id=" << std::setw(5) << *stringIdIt;
    }

    if (auto severityIt = rec[kSeverity].extract<severity_t>())
        strm << " [" << *severityIt << "]";

    strm << " : " << rec[boost::log::expressions::smessage];
}

}    // namespace logger
