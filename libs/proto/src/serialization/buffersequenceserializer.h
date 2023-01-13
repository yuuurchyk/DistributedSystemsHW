#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <boost/asio.hpp>

#include "utils/copymove.h"

namespace Proto
{
class BufferSequenceSerializer
{
    DISABLE_COPY_MOVE(BufferSequenceSerializer)
public:
    /**
     * @brief Construct a new BufferSequenceSerializer object
     *
     * @param seq - calls to serialize() methods populate the @p seq
     *
     * @note serializeWoOwnership() methods don't copy the underlying memory, so make sure
     * your objects live long enough for the buffers to stay valid
     */
    BufferSequenceSerializer(std::vector<boost::asio::const_buffer> &seq);
    ~BufferSequenceSerializer() = default;

    template <typename T>
        requires std::is_integral_v<T> || std::is_enum_v<T>
    void serializeWoOwnership(const T &);

    template <typename T>
        requires std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>
    void serializeWoOwnership(const T &);

private:
    void serializeWoOwnership(const std::byte *, size_t);

    std::vector<boost::asio::const_buffer> &seq_;
};

}    // namespace Proto

#include "buffersequenceserializer_impl.h"
