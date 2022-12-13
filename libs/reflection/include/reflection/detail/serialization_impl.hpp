#pragma once

#include <cstring>
#include <utility>

#include "reflection/serialization.h"

#include "reflection/concepts.h"

namespace Reflection
{
template <TriviallySerializable T>
const T &SerializationContext::add(const T &value)
{
    auto &mem = getContiguousMemory(sizeof(T));
    std::memcpy(&mem, &value, sizeof(T));
    return reinterpret_cast<const T &>(mem);
}

template <Serializable T>
void SerializationContext::serializeAndHold(std::shared_ptr<T> valPtr)
{
    serializeAndHold(std::shared_ptr<const T>{ std::move(valPtr) });
}

template <Serializable T>
void SerializationContext::serializeAndHold(std::shared_ptr<const T> valPtr)
{
    if (valPtr == nullptr)
        return;

    const auto &val = *valPtr;

    allocateChunk(additionalBytesRequired(val));
    seq_.reserve(seq_.size() + buffersRequired(val));

    serializeImpl(val);

    hold(std::move(valPtr));
}

template <typename T>
void SerializationContext::hold(std::shared_ptr<const T> ptr)
{
    if (ptr == nullptr)
        return;

    ptrs_.push_back(std::move(ptr));
}

template <TriviallySerializable T>
size_t SerializationContext::buffersRequired(const T &)
{
    return 1;
}
template <TriviallySerializable T>
size_t SerializationContext::additionalBytesRequired(const T &)
{
    return 0;
}
template <TriviallySerializable T>
void SerializationContext::serializeImpl(const T &val)
{
    seq_.emplace_back(static_cast<const void *>(&val), sizeof(val));
}

// ---------------------------------------------------------

template <HasSerializableTie T>
size_t SerializationContext::buffersRequired(const T &val)
{
    auto res = size_t{};

    auto visit = [&](const auto &val) { res += buffersRequired(val); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, val.tie());

    return res;
}

template <HasSerializableTie T>
size_t SerializationContext::additionalBytesRequired(const T &val)
{
    auto res = size_t{};

    auto visit = [&](const auto &val) { res += additionalBytesRequired(val); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, val.tie());

    return res;
}

template <HasSerializableTie T>
void SerializationContext::serializeImpl(const T &val)
{
    auto visit = [&](const auto &entry) { serializeImpl(entry); };
    std::apply([&](const auto &...entry) { (..., visit(entry)); }, val.tie());
}

// ---------------------------------------------------------

template <TriviallySerializable T>
size_t SerializationContext::buffersRequired(const std::vector<T> &v)
{
    return 2;    // size + entries
}

template <TriviallySerializable T>
size_t SerializationContext::additionalBytesRequired(const std::vector<T> &v)
{
    return sizeof(std::vector<T>::size_type);
}

template <TriviallySerializable T>
void SerializationContext::serializeImpl(const std::vector<T> &v)
{
    serializeImpl(add(v.size()));
    seq_.emplace_back(static_cast<const void *>(v.data()), v.size() * sizeof(T));
}

// ---------------------------------------------------------

template <NonTriviallySerializable T>
size_t SerializationContext::buffersRequired(const std::vector<T> &v)
{
    auto res = size_t{ 1 };
    for (const auto &entry : v)
        res += buffersRequired(entry);
    return res;
}

template <NonTriviallySerializable T>
size_t SerializationContext::additionalBytesRequired(const std::vector<T> &v)
{
    auto res = size_t{ sizeof(typename std::vector<T>::size_type) };

    for (const auto &entry : v)
        res += additionalBytesRequired(entry);

    return res;
}

template <NonTriviallySerializable T>
void SerializationContext::serializeImpl(const std::vector<T> &v)
{
    serializeImpl(add(v.size()));

    for (const auto &entry : v)
        serializeImpl(entry);
}

}    // namespace Reflection
