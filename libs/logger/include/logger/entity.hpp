#pragma once

#include <string>
#include <utility>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>

#include "logger/detail/attributes.h"
#include "logger/detail/entitybase.hpp"
#include "logger/idcounter.hpp"
#include "logger/threadmodel.h"

namespace logger
{

/**
 * Each object of @tparam Entity_t will have its own logger with @tparam
 * threaded_model_t and id provided in ctor
 *
 * @note @tparam Entity_t should have static constexpr std::string_view kEntityName
 * variable
 */
template <typename Entity_t, typename thread_model_t>
class NumIdEntity : public detail::EntityBase<Entity_t, thread_model_t>
{
public:
    NumIdEntity()
    {
        this->entityLogger_.add_attribute(
            detail::attributes::kNumIdAttr,
            boost::log::attributes::make_constant(IdCounter<Entity_t>::getNext()));
    }
};

/**
 * Each object of @tparam Entity_t will have its own logger with @tparam
 * threaded_model_t and id provided in ctor
 *
 * @note @tparam Entity_t should have static constexpr std::string_view kEntityName
 * variable
 */
template <typename Entity_t, typename thread_model_t>
class StringIdEntity : public detail::EntityBase<Entity_t, thread_model_t>
{
public:
    StringIdEntity(std::string id)
    {
        this->entityLogger_.add_attribute(
            detail::attributes::kStringIdAttr,
            boost::log::attributes::make_constant(std::move(id)));
    }
};

}    // namespace logger

#define EN_LOGI BOOST_LOG_SEV(this->entityLogger_, ::logger::Severity::Info)
#define EN_LOGW BOOST_LOG_SEV(this->entityLogger_, ::logger::Severity::Warning)
#define EN_LOGE BOOST_LOG_SEV(this->entityLogger_, ::logger::Severity::Error)
