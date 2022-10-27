#pragma once

#include <string>
#include <string_view>

#include <boost/type_index.hpp>

namespace logger::detail
{
template <typename Entity_t>
class EntityName
{
public:
    static inline const std::string kEntityNameStr{
        boost::typeindex::type_id<Entity_t>().pretty_name()
    };
    static inline const std::string_view kEntityName{ kEntityNameStr };
};

}    // namespace logger::detail
