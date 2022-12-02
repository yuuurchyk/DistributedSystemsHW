#include "proto/serialization.h"

namespace Proto
{
SerializationContext::~SerializationContext()
{
    for (auto &dtor : dtors_)
        dtor();
}

namespace detail
{
    template <>
    void serialize<std::string>(const std::string &s, SerializationContext &context)
    {
        serialize(context.add(s.size()), context);
        context.constBufferSequence.emplace_back(
            s.data(), s.size() * sizeof(std::string::value_type));
    }

}    // namespace detail

}    // namespace Proto
