#include "formatter.hpp"

#include <iomanip>
#include <string_view>
#include <type_traits>

#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions.hpp>
#include <boost/scope_exit.hpp>

#include "logger/detail/attributes.h"
#include "logger/entity.hpp"
#include "logger/idcounter.hpp"
#include "logger/setup.h"
#include "logger/severity.h"

namespace logger
{
void formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &strm)
{
    using namespace detail::attributes;

    if (const auto programNameIt = rec[kProgramNameAttr].extract<program_name_t>())
        strm << "[" << *programNameIt << "]";

    if (const auto threadIdIt = rec[kThreadIdAttr].extract<thread_id_t>())
        strm << " [TRD" << std::hex << ((*threadIdIt).native_id() & (0xfffff)) << "]";

    if (const auto entityNameIt = rec[kEntityNameAttr].extract<entity_name_t>())
    {
        strm << " [" << *entityNameIt;
        BOOST_SCOPE_EXIT(&strm)
        {
            strm << "]";
        }
        BOOST_SCOPE_EXIT_END

        if (const auto numIdIt = rec[kNumIdAttr].extract<num_id_t>())
        {
            static_assert(sizeof(num_id_t) <= 2,
                          "Formatter relies on the following constraint on num_id");
            static constexpr const char *kAlphabet =
                "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,";
            static constexpr size_t kAlphabetSize =
                64;    // ., is added for quicker remainder/division operations

            char idRepresentation[4] = "___";
            auto numId               = *numIdIt;
            for (int i = 2; i >= 0; --i)
            {
                auto rem            = numId % kAlphabetSize;
                idRepresentation[i] = kAlphabet[rem];
                numId               = numId / kAlphabetSize;
            }

            strm << ",id=" << idRepresentation;
        }
        else if (const auto stringIdIt = rec[kStringIdAttr].extract<string_id_t>())
        {
            strm << ",id=" << *stringIdIt;
        }
    }

    {
        const auto fileNameIt   = rec[kFilenameAttr].extract<file_name_t>();
        const auto lineNumberIt = rec[kLineNumberAttr].extract<line_number_t>();

        if (fileNameIt && lineNumberIt)
            strm << " [" << *fileNameIt << ":" << *lineNumberIt << "]";
    }

    if (auto severityIt = rec[kSeverityAttr].extract<::logger::Severity>())
        strm << " [" << *severityIt << "]";

    strm << " : " << rec[boost::log::expressions::smessage];
}

}    // namespace logger
