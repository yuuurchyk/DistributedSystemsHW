#pragma once

#include <optional>
#include <tuple>

#include <boost/asio.hpp>

#include "proto/concepts.h"
#include "proto/proto.h"

namespace Proto
{
template <typename ConstBufferSequence>
class DeserializationPtr;

template <Concepts::Deserializable T, typename ConstBufferSequence>
[[nodiscard]] std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &);

template <typename ConstBufferSequence>
class DeserializationPtr
{
public:
    using Iterator = boost::asio::buffers_iterator<ConstBufferSequence>;

    DeserializationPtr(const ConstBufferSequence &seq);

    /**
     * @return true if buffer has @p count more instances of T
     */
    template <Concepts::TriviallyDeserializable T>
    bool has(size_t count = 1);

    // the behaviour is undefined when has() returns false
    template <Concepts::TriviallyDeserializable T>
    T get();

    // the behaviour is undefined when has() returns false
    template <Concepts::TriviallyDeserializable T>
    std::tuple<Iterator, Iterator> get(size_t count);

    bool atEnd() const noexcept;

private:
    Iterator       current_;
    const Iterator end_;
};

}    // namespace Proto

#include "detail/deserialization_impl.hpp"
