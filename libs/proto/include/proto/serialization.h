#pragma once

#include <memory>
#include <tuple>
#include <vector>

#include <boost/asio.hpp>

#include "proto/proto.h"
#include "utils/copymove.h"

namespace Proto
{
struct SerializationResult;

/**
 * @brief serializes the input @p val into a const buffer sequence
 *
 * @note make sure not to change the address of @p val after the call,
 * since the buffer will become invalid (one solution is to wrap the @p val into a smart
 * pointer)
 *
 * @note Context object holds an additional data needed for serializing, deleting
 * it also makes the buffer sequence invalid
 */
template <Concepts::Serializable T>
[[nodiscard]] SerializationResult serialize(const T &val);

class SerializationContext
{
public:
    DISABLE_COPY_DEFAULT_MOVE(SerializationContext)

    // specifies the size of the first chunk
    SerializationContext(size_t bytesNum = kDefaultChunkSize);

    template <std::integral T>
    const T &add(T value);

private:
    // optimize number of memory allocations
    static constexpr size_t kDefaultChunkSize{ sizeof(size_t) * 4 };
    static_assert(kDefaultChunkSize > 0);

    std::byte &getContiguousMemory(size_t bytesNum);
    void       allocateChunk(size_t bytesNum);

    std::vector<std::unique_ptr<std::byte[]>> chunks_;
    std::byte                                *currentChunk_;
    std::byte                                *currentChunkEnd_;
};

struct SerializationResult
{
    std::vector<boost::asio::const_buffer> constBufferSequence;
    SerializationContext                   context;
};

}    // namespace Proto

#include "detail/serialization_impl.hpp"
