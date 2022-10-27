#include "logger/translationunit.h"

#include <utility>

#include <boost/log/attributes/constant.hpp>

#include "logger/detail/attributes.h"

namespace logger
{
TranslationUnitLogger::TranslationUnitLogger(std::string_view moduleName)
{
    add_attribute(detail::tag::attr_entity_name::get_name(),
                  boost::log::attributes::make_constant(std::move(moduleName)));
}

}    // namespace logger
