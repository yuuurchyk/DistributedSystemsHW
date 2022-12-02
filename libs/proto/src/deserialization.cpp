#include "proto/deserialization.h"

namespace Proto::detail
{
std::optional<std::string> Deserializer<std::string>::deserialize(DeserializationPtr &ptr)
{
    static_assert(std::is_pod_v<std::string::value_type>);

    const auto sizePtr = ptr.consume<std::string::size_type>();
    if (sizePtr == nullptr)
        return {};

    const auto size = *sizePtr;
    auto       res  = std::string{};
    res.reserve(size);

    auto dataPtr = ptr.consume<std::string::value_type>(size);
    if (dataPtr == nullptr)
        return {};

    res.resize(size);
    std::memcpy(res.data(), dataPtr, sizeof(std::string::value_type) * size);

    return res;
}

}    // namespace Proto::detail
