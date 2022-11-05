#include "uptime.h"

#include <boost/log/attributes/attribute_value_impl.hpp>

using namespace boost::log;
using namespace std::chrono;

namespace logger
{

const steady_clock::time_point kUpTimepoint{ steady_clock::now() };

attribute_value UptimeMsImpl::get_value()
{
    const auto now = steady_clock::now();

    size_t res = duration_cast<milliseconds>(now - kUpTimepoint).count();

    return attributes::make_attribute_value(res);
}

UptimeMs::UptimeMs() : boost::log::attribute{ new UptimeMsImpl } {}

UptimeMs::UptimeMs(const attributes::cast_source &source)
    : attribute{ source.as<UptimeMsImpl>() }
{
}

}    // namespace logger
