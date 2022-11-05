#pragma once

#include <boost/parameter/keyword.hpp>

#include "logger/detail/attributes.h"
#include "logger/detail/keyvaluefeature.hpp"

namespace logger::detail
{
namespace keywords
{
    BOOST_PARAMETER_KEYWORD(tags, CodeFilename)
    BOOST_PARAMETER_KEYWORD(tags, CodeLineNumber)

}    // namespace keywords

struct CodeFilenameKeywordGetter
{
    auto &operator()() const { return keywords::CodeFilename; }
};
struct CodeFilenameAttrNameGetter
{
    auto operator()() const { return attributes::kCodeFilename; }
};

struct CodeLineNumberKeywordGetter
{
    auto &operator()() const { return keywords::CodeLineNumber; }
};
struct CodeLineNumberAttrNameGetter
{
    auto operator()() const { return attributes::kCodeLineNumber; }
};

}    // namespace logger::detail
