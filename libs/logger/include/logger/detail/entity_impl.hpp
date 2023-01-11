#pragma once

#include <string_view>

#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/channel_feature.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/parameter/keyword.hpp>
#include <boost/type_index.hpp>

#include "logger/detail/attributes.h"
#include "logger/detail/codelocation.hpp"
#include "logger/detail/keyvaluefeature.hpp"
#include "logger/severity.h"

namespace logger::detail
{
namespace keywords
{
    BOOST_PARAMETER_KEYWORD(tags, NumId)
    BOOST_PARAMETER_KEYWORD(tags, StringId)
}    // namespace keywords

template <typename Entity_t>
class EntityName
{
public:
    // clang-format off
    static inline const std::string kEntityNameStr_{ boost::typeindex::type_id<Entity_t>().pretty_name() };
    static inline const std::string_view kEntityName{ kEntityNameStr_ };
    // clang-format on
};

struct NumIdKeywordGetter
{
    auto &operator()() const { return keywords::NumId; }
};
struct NumIdAttrNameGetter
{
    auto operator()() const { return attributes::kNumId; }
};

struct StringIdKeywordGetter
{
    auto &operator()() const { return keywords::StringId; }
};
struct StringIdAttrNameGetter
{
    auto operator()() const { return attributes::kStringId; }
};

class EntityLogger
    : public boost::log::sources::basic_composite_logger<
          char,
          EntityLogger,
          boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>,
          boost::log::sources::features<
              boost::log::sources::severity<Severity>,
              boost::log::sources::channel<std::string_view>,
              detail::KeyValue<CodeFilenameKeywordGetter, CodeFilenameAttrNameGetter, attributes::code_file_name_t>,
              detail::
                  KeyValue<CodeLineNumberKeywordGetter, CodeLineNumberAttrNameGetter, attributes::code_line_number_t>>>
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS_TEMPLATE(EntityLogger)
};

class NumIdEntityLogger
    : public boost::log::sources::basic_composite_logger<
          char,
          NumIdEntityLogger,
          boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>,
          boost::log::sources::features<
              boost::log::sources::severity<Severity>,
              boost::log::sources::channel<std::string_view>,
              detail::KeyValue<NumIdKeywordGetter, NumIdAttrNameGetter, attributes::num_id_t>,
              detail::KeyValue<CodeFilenameKeywordGetter, CodeFilenameAttrNameGetter, attributes::code_file_name_t>,
              detail::
                  KeyValue<CodeLineNumberKeywordGetter, CodeLineNumberAttrNameGetter, attributes::code_line_number_t>>>
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS_TEMPLATE(NumIdEntityLogger)
};

class StringIdEntityLogger
    : public boost::log::sources::basic_composite_logger<
          char,
          StringIdEntityLogger,
          boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>,
          boost::log::sources::features<
              boost::log::sources::severity<Severity>,
              boost::log::sources::channel<std::string_view>,
              detail::KeyValue<StringIdKeywordGetter, StringIdAttrNameGetter, attributes::string_id_t>,
              detail::KeyValue<CodeFilenameKeywordGetter, CodeFilenameAttrNameGetter, attributes::code_file_name_t>,
              detail::
                  KeyValue<CodeLineNumberKeywordGetter, CodeLineNumberAttrNameGetter, attributes::code_line_number_t>>>
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS_TEMPLATE(StringIdEntityLogger)
};

}    // namespace logger::detail
