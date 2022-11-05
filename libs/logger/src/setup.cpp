#include "logger/logger.h"

#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/counter.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/core.hpp>
#include <boost/log/keywords/order.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/shared_ptr.hpp>

#include <functional>
#include <iostream>
#include <utility>

#include "formatter.hpp"
#include "logger/detail/attributes.h"

namespace attrs    = boost::log::attributes;
namespace logging  = boost::log;
namespace sinks    = boost::log::sinks;
namespace keywords = boost::log::keywords;

using backend_t = sinks::text_ostream_backend;
using sink_t    = sinks::asynchronous_sink<
    backend_t,
    sinks::unbounded_ordering_queue<
        logging::attribute_value_ordering<unsigned int, std::less<unsigned int>>>>;

namespace
{
boost::shared_ptr<sink_t> sink;

}    // namespace

namespace logger
{
void setup(std::string programName)
{
    using namespace detail::attributes;

    if (::sink != nullptr)
        return;

    logging::core::get()->add_global_attribute(
        kProgramName, boost::log::attributes::make_constant(std::move(programName)));
    logging::core::get()->add_global_attribute(kRecordId, attrs::counter<unsigned int>());
    logging::core::get()->add_global_attribute(kThreadId, attrs::current_thread_id());

    ::sink.reset(new sink_t{ boost::make_shared<backend_t>(),
                             keywords::order = logging::make_attr_ordering<unsigned int>(
                                 kRecordId, std::less<unsigned int>()) });

    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter{}));
    sink->set_formatter(&::logger::formatter);

    boost::log::core::get()->add_sink(sink);
}

void teardown()
{
    if (::sink == nullptr)
        return;

    ::sink->stop();
    ::sink->flush();
    ::sink.reset();
}

}    // namespace logger
