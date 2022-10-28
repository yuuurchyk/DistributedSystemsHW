#pragma once

#include <string>
#include <type_traits>
#include <utility>

#include <boost/log/utility/manipulators/add_value.hpp>

#include "logger/detail/attributes.h"
#include "logger/detail/entityname.hpp"
#include "logger/detail/idcounter.hpp"
#include "logger/logger.h"

namespace logger
{
/**
 * @brief
 *
 * @tparam Entity_t
 *
 * @note make sure you derive from this class only once
 */
template <typename Entity_t>
class IdEntity
{
public:
    IdEntity() : loggerId_{ detail::IdCounter<Entity_t>::getNext() } {}

    IdEntity(const IdEntity &)            = delete;
    IdEntity(IdEntity &&)                 = default;
    IdEntity &operator=(const IdEntity &) = delete;
    IdEntity &operator=(IdEntity &&)      = default;

    ~IdEntity() = default;

protected:
    detail::logger_id_t loggerId() const noexcept { return loggerId_; }

private:
    const detail::logger_id_t loggerId_;
};

}    // namespace logger

#define _ID_LOGIMPL(severity)                                                            \
    _LOGIMPL(severity)                                                                   \
        << ::boost::log::add_value(                                                      \
               ::logger::detail::attributes::kEntityNameAttr,                            \
               ::logger::detail::EntityName<std::decay_t<decltype(*this)>>::kEntityName) \
        << ::boost::log::add_value(::logger::detail::attributes::kIdAttr,                \
                                   this->loggerId())

#define ID_LOGI _ID_LOGIMPL(Info)
#define ID_LOGW _ID_LOGIMPL(Warning)
#define ID_LOGE _ID_LOGIMPL(Error)