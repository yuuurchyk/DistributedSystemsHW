#include <gtest/gtest.h>

#include "proto/proto.h"
#include "proto/serialization.h"

namespace
{
using namespace Proto;

TEST(Serialization, PODs)
{
    const int    a = 1;
    const size_t b = 2;

    auto context = SerializationContext{};
    detail::serialize(a, context);
    detail::serialize(b, context);

    const auto &seq = context.constBufferSequence;
    ASSERT_EQ(seq.size(), 2);
    ASSERT_EQ(seq[0].size(), sizeof(int));
    ASSERT_EQ(seq[1].size(), sizeof(size_t));
    ASSERT_EQ(&a, seq[0].data());
    ASSERT_EQ(&b, seq[1].data());
    ASSERT_EQ(a, *reinterpret_cast<const int *>(seq[0].data()));
    ASSERT_EQ(b, *reinterpret_cast<const size_t *>(seq[1].data()));

    const auto &ra = context.add(a);
    const auto &rb = context.add(b);

    ASSERT_NE(&ra, &a);
    ASSERT_NE(&rb, &b);
}

TEST(Serialization, String)
{
    const auto s = std::string{ "abc" };

    auto context = SerializationContext{};
    detail::serialize(s, context);

    const auto &seq = context.constBufferSequence;

    ASSERT_EQ(seq.size(), 2);
    ASSERT_EQ(seq[0].size(), sizeof(std::string::size_type));
    ASSERT_EQ(seq[1].size(), s.size());
    ASSERT_EQ(s.size(), *reinterpret_cast<const std::string::size_type *>(seq[0].data()));
    ASSERT_EQ(s.data(), seq[1].data());
}

TEST(Serialization, Tuple)
{
    const auto t = std::make_tuple(1, 2, 3);

    auto context = SerializationContext{};
    detail::serialize(t, context);
}

TEST(Serialization, TupleOfRefs)
{
    auto a = 1;
    auto b = size_t{ 2 };

    const auto t = std::make_tuple(std::ref(a), std::ref(b));

    auto context = SerializationContext{};
    detail::serialize(t, context);

    const auto &seq = context.constBufferSequence;

    ASSERT_EQ(seq.size(), 2);
    ASSERT_EQ(seq[0].size(), sizeof(int));
    ASSERT_EQ(seq[1].size(), sizeof(size_t));
    ASSERT_EQ(seq[0].data(), &a);
    ASSERT_EQ(seq[1].data(), &b);
}

TEST(Serialization, TupleOfReferences)
{
    auto a = 1;
    auto b = size_t{ 2 };

    const auto t = std::tie(a, b);

    auto context = SerializationContext{};
    detail::serialize(t, context);

    const auto &seq = context.constBufferSequence;
}

TEST(Serialization, OtherCombinations)
{
    auto v1 = std::vector<std::optional<std::string>>{};
    auto v2 = std::optional<std::vector<std::optional<std::string>>>{};
    auto v3 = std::optional<std::reference_wrapper<const std::string>>{};

    auto context = SerializationContext{};
    detail::serialize(v1, context);
    detail::serialize(v2, context);
    detail::serialize(v3, context);
}

TEST(Serialization, CustomClass)
{
    auto message = Message{ getCurrentTimestamp(), "sample message" };

    auto context = SerializationContext{};
    detail::serialize(message, context);
}

TEST(Serialization, Request_AddMessage)
{
    const auto timestamp = getCurrentTimestamp();

    auto request = Request::AddMessage{ Message{ timestamp, "sample message" } };

    auto context = serialize(std::move(request));

    const auto &bufferSequence = context->constBufferSequence;
    ASSERT_EQ(bufferSequence.size(),
              5);    // event type, opcode, timestamp, string size, string itself
    ASSERT_EQ(bufferSequence[0].size(), sizeof(EventType));
    ASSERT_EQ(bufferSequence[1].size(), sizeof(OpCode));
    ASSERT_EQ(bufferSequence[2].size(), sizeof(Timestamp_t));
    ASSERT_EQ(bufferSequence[3].size(), sizeof(std::string::size_type));
    ASSERT_EQ(bufferSequence[4].size(),
              (sizeof("sample message") - 1) * sizeof(std::string::value_type));
}

TEST(Serialization, OtherProtoClasses)
{
    {
        auto request = Request::GetMessages{};
        auto context = serialize(std::move(request));
    }
    {
        auto response     = Response::GetMessages{};
        response.status   = Response::GetMessages::Status::OK;
        response.messages = { Message{ getCurrentTimestamp(), "sample message 1" } };
        auto context      = serialize(std::move(response));
    }
    {
        auto response = Response::AddMessage{};
        auto context  = serialize(std::move(response));
    }
}

}    // namespace
