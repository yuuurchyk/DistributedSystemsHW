#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include <boost/log/attributes/current_thread_id.hpp>

#include "logger/detail/severity.h"

namespace logger::detail::attributes
{
constexpr const char *kProgramName{ "attr_program_name" };
using program_name_t = std::string;

extern const char *kThreadId;
using thread_id_t = boost::log::thread_id;

extern const char *kRecordId;

extern const char *kSeverity;
using severity_t = Severity;

constexpr const char *kCodeFilename{ "code_attr_file_name" };
using code_file_name_t = std::string_view;
constexpr const char *kCodeLineNumber{ "code_attr_line_number" };
using code_line_number_t = size_t;

extern const char *kChannel;
using channel_t = std::string_view;

constexpr const char *kNumId{ "attr_num_id" };
using num_id_t = size_t;
constexpr const char *kStringId{ "attr_string_id" };
using string_id_t = std::string;

constexpr const char *kUpTimeMs{ "attr_uptime_ms" };
using uptime_ms_t = size_t;

}    // namespace logger::detail::attributes
