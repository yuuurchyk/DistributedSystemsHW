#pragma once

#include <atomic>
#include <cstddef>
#include <string>
#include <utility>

#include <boost/log/sources/record_ostream.hpp>

#include "utils/copymove.h"

#include "logger/detail/attributes.h"
#include "logger/detail/entity_impl.hpp"
#include "logger/detail/extractbasename.hpp"

namespace logger
{
/**
 * @brief adds name of the class of @tparam Entity_t to logs
 */
template <typename Entity_t>
class Entity
{
protected:
    static inline detail::EntityLogger d3WzdZnJLogger_{ (
        ::boost::log::keywords::channel = detail::EntityName<Entity_t>::kEntityName) };

    auto d3WzdZnJOpenRecord_(
        detail::Severity                       severity,
        detail::attributes::code_file_name_t   codeFileName,
        detail::attributes::code_line_number_t codeLineNumber)
    {
        return this->d3WzdZnJLogger_.open_record((BOOST_PP_SEQ_ENUM(
            (::boost::log::keywords::severity = severity)(detail::keywords::CodeFilename = std::move(codeFileName))(
                detail::keywords::CodeLineNumber = std::move(codeLineNumber)))));
    }
};

/**
 * @brief adds name of the class of @tparam Entity_t + numerical id to logs
 */
template <typename Entity_t>
class NumIdEntity
{
public:
    DISABLE_COPY_DEFAULT_MOVE(NumIdEntity);

    NumIdEntity() : id_{ idCounter_.fetch_add(1, std::memory_order_relaxed) } {}
    NumIdEntity(detail::attributes::num_id_t id) : id_{ id } {}

protected:
    static inline detail::NumIdEntityLogger d3WzdZnJLogger_{ (
        ::boost::log::keywords::channel = detail::EntityName<Entity_t>::kEntityName) };

    auto d3WzdZnJOpenRecord_(
        detail::Severity                       severity,
        detail::attributes::code_file_name_t   codeFileName,
        detail::attributes::code_line_number_t codeLineNumber)
    {
        return this->d3WzdZnJLogger_.open_record((BOOST_PP_SEQ_ENUM((::boost::log::keywords::severity = severity)(
            detail::keywords::NumId = id_)(detail::keywords::CodeFilename = std::move(codeFileName))(
            detail::keywords::CodeLineNumber = std::move(codeLineNumber)))));
    }

    detail::attributes::num_id_t loggerId() const noexcept { return id_; }

private:
    static inline std::atomic<detail::attributes::num_id_t> idCounter_{};

    detail::attributes::num_id_t id_;
};

/**
 * @brief adds name of the class of @tparam Entity_t + string id to logs
 */
template <typename Entity_t>
class StringIdEntity
{
public:
    DISABLE_COPY_DEFAULT_MOVE(StringIdEntity);

    StringIdEntity(std::string id) : id_{ std::move(id) } {}

protected:
    static inline detail::StringIdEntityLogger d3WzdZnJLogger_{ (
        ::boost::log::keywords::channel = detail::EntityName<Entity_t>::kEntityName) };

    auto d3WzdZnJOpenRecord_(
        detail::Severity                       severity,
        detail::attributes::code_file_name_t   codeFileName,
        detail::attributes::code_line_number_t codeLineNumber)
    {
        return this->d3WzdZnJLogger_.open_record((BOOST_PP_SEQ_ENUM((::boost::log::keywords::severity = severity)(
            detail::keywords::StringId = id_)(detail::keywords::CodeFilename = std::move(codeFileName))(
            detail::keywords::CodeLineNumber = std::move(codeLineNumber)))));
    }

    const std::string &loggerId() const noexcept { return id_; }

private:
    std::string id_;
};

}    // namespace logger

#define _EN_LOG_IMPL(sev, rec_var)                                             \
    for (::boost::log::record rec_var = this->d3WzdZnJOpenRecord_(            \
             ::logger::detail::Severity::sev,                                 \
             ::logger::detail::extractBaseName(std::string_view{ __FILE__ }), \
             ::logger::detail::attributes::code_line_number_t{ __LINE__ });   \
         !!rec_var;)                                                          \
    ::boost::log::aux::make_record_pump((this->d3WzdZnJLogger_), rec_var).stream()

#define EN_LOGD _EN_LOG_IMPL(Debug, BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_en_log_record_))
#define EN_LOGI _EN_LOG_IMPL(Info, BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_en_log_record_))
#define EN_LOGW _EN_LOG_IMPL(Warning, BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_en_log_record_))
#define EN_LOGE _EN_LOG_IMPL(Error, BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_en_log_record_))
