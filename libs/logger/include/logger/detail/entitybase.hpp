#pragma once

#include <string_view>
#include <type_traits>
#include <utility>

#include <boost/log/sources/severity_logger.hpp>
#include <boost/type_index.hpp>

#include "logger/detail/attributes.h"
#include "logger/severity.h"
#include "logger/threadmodel.h"

namespace logger::detail
{
template <typename Entity_t, typename thread_model_t>
class EntityBase
{
    static_assert(std::is_same_v<thread_model_t, single_thread_model> ||
                      std::is_same_v<thread_model_t, multi_thread_model>,
                  "Wrong logger thread model");

    using logger_t =
        std::conditional_t<std::is_same_v<thread_model_t, single_thread_model>,
                           boost::log::sources::severity_logger<::logger::Severity>,
                           boost::log::sources::severity_logger_mt<::logger::Severity>>;

public:
    EntityBase()
    {
        static const std::string kEntityName{
            boost::typeindex::type_id<Entity_t>().pretty_name()
        };
        static const boost::log::attributes::constant<std::string_view> kEntityNameAttr{
            kEntityName
        };

        entityLogger_.add_attribute(detail::attributes::kEntityNameAttr, kEntityNameAttr);
    }

protected:
    logger_t entityLogger_{};

private:
    EntityBase(const EntityBase &)            = delete;
    EntityBase(EntityBase &&)                 = delete;
    EntityBase &operator=(const EntityBase &) = delete;
    EntityBase &operator=(EntityBase &&)      = delete;
};

}    // namespace logger::detail
