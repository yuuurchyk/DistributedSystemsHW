#pragma once

#include <string>
#include <string_view>

#include <boost/log/attributes/current_thread_id.hpp>

#include "logger/detail/idcounter.hpp"
#include "logger/detail/severity.h"

namespace logger::detail::attributes
{
constexpr const char *kProgramNameAttr{ "attr_program_name" };
using program_name_t = std::string;

constexpr const char *kEntityNameAttr{ "attr_entity_name" };
using entity_name_t = std::string_view;

constexpr const char *kIdAttr{ "attr_id" };
using logger_id_t = logger::detail::logger_id_t;

constexpr const char *kFilenameAttr{ "attr_file_name" };
using file_name_t = std::string_view;

constexpr const char *kLineNumberAttr{ "attr_line_number" };
using line_number_t = int;

constexpr const char *kThreadIdAttr{ "ThreadID" };
using thread_id_t = boost::log::thread_id;

constexpr const char *kSeverityAttr{ "Severity" };
constexpr const char *kRecordIdAttr{ "RecordID" };

}    // namespace logger::detail::attributes
