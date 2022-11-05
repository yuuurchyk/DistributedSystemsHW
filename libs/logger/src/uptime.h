#pragma once

#include <chrono>

#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_cast.hpp>
#include <boost/log/attributes/attribute_value.hpp>

namespace logger
{

extern const std::chrono::steady_clock::time_point kUpTimepoint;

class UptimeMsImpl : public boost::log::attribute::impl
{
public:
    boost::log::attribute_value get_value() override;
};

class UptimeMs : public boost::log::attribute
{
public:
    UptimeMs();
    explicit UptimeMs(const boost::log::attributes::cast_source &);
};

}    // namespace logger
