#include <cstddef>
#include <cstring>
#include <memory>

#include <gtest/gtest.h>

#include "proto/deserialization.h"
#include "proto/proto.h"
#include "proto/serialization.h"
#include "utils/copymove.h"

namespace
{
using namespace Proto;

/**
 * @brief copies input ConstBufferSequence into a contiguous array of memory
 */
class MemoryChunk
{
    DISABLE_COPY_MOVE(MemoryChunk)

public:
    MemoryChunk(const std::deque<boost::asio::const_buffer> &constBufferSequence)
    {
        for (const auto &piece : constBufferSequence)
            size_ += piece.size();

        rawMemory_.reset(new (std::nothrow) std::byte[size_]);

        auto ptr = rawMemory_.get();
        for (const auto &piece : constBufferSequence)
        {
            std::memcpy(ptr, piece.data(), piece.size());
            ptr += piece.size();
        }
    }

    const void *memory() const { return rawMemory_.get(); }
    size_t      size() const noexcept { return size_; }

private:
    size_t                       size_{};
    std::unique_ptr<std::byte[]> rawMemory_{};
};

TEST(Deserialization, PODs)
{
    const auto a = 1;
    const auto b = size_t{ 5 };
    const auto c = 3;

    auto context = SerializationContext{};
    detail::serialize(a, context);
    detail::serialize(b, context);
    detail::serialize(c, context);

    const auto chunk = MemoryChunk{ context.constBufferSequence };
    auto       ptr   = detail::DeserializationPtr{ chunk.memory(), chunk.size() };

    ASSERT_EQ(chunk.size(), 2 * sizeof(int) + sizeof(size_t));

    const auto resa = detail::Deserializer<int>::deserialize(ptr);
    const auto resb = detail::Deserializer<size_t>::deserialize(ptr);
    const auto resc = detail::Deserializer<int>::deserialize(ptr);

    ASSERT_TRUE(resa.has_value());
    ASSERT_TRUE(resb.has_value());
    ASSERT_TRUE(resc.has_value());

    ASSERT_EQ(resa.value(), 1);
    ASSERT_EQ(resb.value(), 5);
    ASSERT_EQ(resc.value(), 3);
}

TEST(Deserialization, PODsInvalidData)
{
    const auto a = 5;
    const char b = 'a';

    auto context = SerializationContext{};
    detail::serialize(a, context);
    detail::serialize(b, context);

    const auto chunk = MemoryChunk{ context.constBufferSequence };
    auto       ptr   = detail::DeserializationPtr{ chunk.memory(), chunk.size() };

    const auto resa = detail::Deserializer<int>::deserialize(ptr);
    const auto resb = detail::Deserializer<size_t>::deserialize(ptr);
    const auto resc = detail::Deserializer<int>::deserialize(ptr);

    ASSERT_TRUE(resa.has_value());
    ASSERT_FALSE(resb.has_value());
    ASSERT_FALSE(resc.has_value());

    ASSERT_EQ(resa.value(), 5);
}

TEST(Deserialization, Vector)
{
    auto v = std::vector<std::optional<std::string>>{};
    v.push_back({});
    v.push_back("abc");
    v.push_back("defg");
    v.push_back({});

    auto context = SerializationContext{};
    detail::serialize(v, context);

    const auto chunk = MemoryChunk{ context.constBufferSequence };
    auto       ptr   = detail::DeserializationPtr{ chunk.memory(), chunk.size() };

    const auto res =
        detail::Deserializer<std::vector<std::optional<std::string>>>::deserialize(ptr);

    ASSERT_TRUE(res.has_value());
    auto &resVal = res.value();
    ASSERT_EQ(resVal.size(), 4);
    ASSERT_EQ(resVal[0], std::optional<std::string>{});
    ASSERT_EQ(resVal[1], "abc");
    ASSERT_EQ(resVal[2], "defg");
    ASSERT_EQ(resVal[3], std::optional<std::string>{});
}

TEST(Deserialization, CustomClass)
{
    const auto message = Message{ 123, "hello" };

    auto context = SerializationContext{};
    detail::serialize(message, context);

    const auto chunk = MemoryChunk{ context.constBufferSequence };

    const auto res = deserialize<Message>(chunk.memory(), chunk.size());

    ASSERT_TRUE(res.has_value());

    const auto &restoredMessage = res.value();

    ASSERT_EQ(restoredMessage.timestamp, message.timestamp);
    ASSERT_EQ(restoredMessage.message, message.message);
}

}    // namespace
