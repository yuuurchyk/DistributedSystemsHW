#pragma once

#include <string>
#include <string_view>

#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions/keyword.hpp>

#include "logger/idcounter.hpp"
#include "logger/severity.h"

namespace logger::detail
{
BOOST_LOG_ATTRIBUTE_KEYWORD(attr_program_name, "attr_program_name", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(attr_entity_name, "attr_entity_name", std::string_view)
BOOST_LOG_ATTRIBUTE_KEYWORD(attr_num_id, "attr_num_id", num_id_t)
BOOST_LOG_ATTRIBUTE_KEYWORD(attr_string_id, "attr_string_id", std::string)

constexpr const char *kThreadIdName{ "ThreadID" };
using thread_id_t = boost::log::attributes::current_thread_id;

constexpr const char *kSeverityName{ "Severity" };
constexpr const char *kRecordIdName{ "RecordID" };

}    // namespace logger::detail
