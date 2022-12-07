#pragma once

#include <optional>
#include <tuple>

#include <boost/asio.hpp>

#include "reflection/concepts.h"
#include "utils/copymove.h"

namespace Reflection
{
template <typename ConstBufferSequence>
class DeserializationContext
{
    DISABLE_COPY_MOVE(DeserializationContext)
public:
    DeserializationContext(const ConstBufferSequence &seq);

    template <Deserializable T>
    std::optional<T> deserialize();

    bool atEnd() const noexcept;

private:
    using Iterator = boost::asio::buffers_iterator<ConstBufferSequence>;

    /**
     * @return true if buffer has @p count more instances of T
     */
    template <TriviallyDeserializable T>
    bool has(size_t count = 1);

    // the behaviour is undefined when has() returns false
    template <TriviallyDeserializable T>
    T get();

    // the behaviour is undefined when has() returns false
    template <TriviallyDeserializable T>
    std::tuple<Iterator, Iterator> get(size_t count);

    template <typename T>
    struct Tag
    {
    };

    template <TriviallyDeserializable T>
    [[nodiscard]] std::optional<T> deserializeImpl(Tag<T>);
    template <HasDeserializableTie T>
    [[nodiscard]] std::optional<T> deserializeImpl(Tag<T>);

    template <TriviallyDeserializable T>
    [[nodiscard]] std::optional<std::vector<T>> deserializeImpl(Tag<std::vector<T>>);
    template <NonTriviallyDeserializable T>
    [[nodiscard]] std::optional<std::vector<T>> deserializeImpl(Tag<std::vector<T>>);

    [[nodiscard]] std::optional<std::string> deserializeImpl(Tag<std::string>);

    Iterator       current_;
    const Iterator end_;
};

}    // namespace Reflection

#include "detail/deserialization_impl.hpp"
