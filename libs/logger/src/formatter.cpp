#include "formatter.hpp"

#include <iomanip>
#include <string_view>
#include <type_traits>

#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions.hpp>

#include "logger/detail/attributes.h"
#include "logger/entity.hpp"
#include "logger/idcounter.hpp"
#include "logger/setup.h"
#include "logger/severity.h"

namespace
{
template <typename T>
void trim(const T &s, size_t trimmedSize, boost::log::formatting_ostream &strm)
{
    static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);

    if (s.size() <= trimmedSize)
    {
        strm << std::setw(trimmedSize) << s;
    }
    else
    {
        if (trimmedSize > 3)
        {
            auto view = std::string_view{ s.data(), trimmedSize - 3 };
            strm << view << "...";
        }
        else
        {
            auto view = std::string_view{ s.data(), trimmedSize };
            strm << view;
        }
    }
}

}    // namespace

namespace logger
{
void formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &strm)
{
    using namespace detail::attributes;

    if (auto programNameIt = rec[kProgramNameAttr].extract<program_name_t>())
    {
        strm << "[";
        trim(*programNameIt, 6, strm);
        strm << "]";
    }
    else
    {
        strm << std::setw(8) << "";
    }

    if (auto threadIdIt = rec[kThreadIdAttr].extract<thread_id_t>())
    {
        strm << " [TRD" << std::hex << ((*threadIdIt).native_id() & (0xfffff)) << "]";
    }
    else
    {
        strm << std::setw(11) << "";
    }

    if (auto entityIt = rec[kEntityNameAttr].extract<entity_name_t>())
    {
        strm << " [";
        trim(*entityIt, 15, strm);
        strm << "]";
    }
    else
    {
        strm << std::setw(18) << "";
    }

    static_assert(sizeof(num_id_t) <= 2,
                  "Formatter relies on the following constraint on num_id");
    if (auto numIdIt = rec[kNumIdAttr].extract<num_id_t>())
    {
        static constexpr const char *kAlphabet =
            "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,";
        static constexpr size_t kAlphabetSize =
            64;    // for quicker remainder/division operations

        strm << " [ id=";

        char idRepresentation[4] = "___";
        auto numId               = *numIdIt;
        for (int i = 2; i >= 0; --i)
        {
            auto rem            = numId % kAlphabetSize;
            idRepresentation[i] = kAlphabet[rem];
            numId               = numId / kAlphabetSize;
        }

        strm << idRepresentation << " ]";
    }
    else if (auto stringIdIt = rec[kStringIdAttr].extract<string_id_t>())
    {
        strm << " [id=";
        trim(*stringIdIt, 5, strm);
        strm << "]";
    }
    else
    {
        strm << std::setw(11) << "";
    }

    if (auto severityIt = rec[kSeverityAttr].extract<::logger::Severity>())
    {
        strm << " [" << *severityIt << "]";
    }
    else
    {
        strm << std::setw(7) << "";
    }

    strm << " : " << rec[boost::log::expressions::smessage];
}

}    // namespace logger
