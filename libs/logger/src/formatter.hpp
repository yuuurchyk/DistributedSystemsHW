#pragma once

#include <boost/log/core/record_view.hpp>
#include <boost/log/utility/formatting_ostream.hpp>

namespace logger
{
void formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &strm);

}    // namespace logger
