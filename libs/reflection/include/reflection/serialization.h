#pragma once

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <boost/asio.hpp>

#include "reflection/concepts.h"
#include "utils/copymove.h"

namespace Reflection
{
class SerializationContext
{
    DISABLE_COPY_MOVE(SerializationContext)

public:
    SerializationContext()  = default;
    ~SerializationContext() = default;

    const std::vector<boost::asio::const_buffer> &constBufferSequence() const noexcept;

    /**
     * @brief serializes the input @p val into a const buffer sequence
     *
     * @note SerializationContext object holds an additional data needed for serializing,
     * deleting it also makes the buffer sequence invalid
     *
     * @note lifetime of @p val is extended
     *
     * @note it is expected that @p val will not change after the serialization
     */
    template <Serializable T>
    void serializeAndHold(std::shared_ptr<T> val);
    template <Serializable T>
    void serializeAndHold(std::shared_ptr<const T> val);

    /**
     * @brief extends the lifetime of @p ptr
     */
    template <typename T>
    void hold(std::shared_ptr<const T> ptr);

private:
    /**
     * @brief allocates @p value on the heap and stores it in SerializationContext
     * object
     */
    template <TriviallySerializable T>
    const T &add(const T &value);

    std::byte &getContiguousMemory(size_t bytesNum);
    void       allocateChunk(size_t bytesNum);

    template <TriviallySerializable T>
    [[nodiscard]] static size_t buffersRequired(const T &);
    template <TriviallySerializable T>
    [[nodiscard]] static size_t additionalBytesRequired(const T &);
    template <TriviallySerializable T>
    void serializeImpl(const T &);

    template <HasSerializableTie T>
    [[nodiscard]] static size_t buffersRequired(const T &);
    template <HasSerializableTie T>
    [[nodiscard]] static size_t additionalBytesRequired(const T &);
    template <HasSerializableTie T>
    void serializeImpl(const T &);

    template <TriviallySerializable T>
    [[nodiscard]] static size_t buffersRequired(const std::vector<T> &);
    template <TriviallySerializable T>
    [[nodiscard]] static size_t additionalBytesRequired(const std::vector<T> &);
    template <TriviallySerializable T>
    void serializeImpl(const std::vector<T> &);

    template <NonTriviallySerializable T>
    [[nodiscard]] static size_t buffersRequired(const std::vector<T> &);
    template <NonTriviallySerializable T>
    [[nodiscard]] static size_t additionalBytesRequired(const std::vector<T> &);
    template <NonTriviallySerializable T>
    void serializeImpl(const std::vector<T> &);

    static_assert(Serializable<std::string>);
    [[nodiscard]] static size_t buffersRequired(const std::string &);
    [[nodiscard]] static size_t additionalBytesRequired(const std::string &);
    void                        serializeImpl(const std::string &);

    std::vector<boost::asio::const_buffer> seq_;
    std::vector<std::any>                  ptrs_;

    std::vector<std::unique_ptr<std::byte[]>> chunks_;
    std::byte                                *currentChunk_{};
    std::byte                                *currentChunkEnd_{};
};

}    // namespace Reflection

#include "detail/serialization_impl.hpp"
