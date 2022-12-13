#include "logger/detail/attributes.h"

#include <boost/log/detail/default_attribute_names.hpp>

namespace
{
namespace names = boost::log::aux::default_attribute_names;

const std::string kThreadIdStr{ names::thread_id().string() };
const std::string kRecordIdStr{ names::line_id().string() };
const std::string kSeverityStr{ names::severity().string() };
const std::string kChannelStr{ names::channel().string() };
}    // namespace

namespace logger::detail::attributes
{
const char *kThreadId{ kThreadIdStr.data() };
const char *kRecordId{ kRecordIdStr.data() };
const char *kSeverity{ kSeverityStr.data() };
const char *kChannel{ kChannelStr.data() };

}    // namespace logger::detail::attributes
