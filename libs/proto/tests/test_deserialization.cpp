#include <gtest/gtest.h>

#include "proto/deserialization.h"
#include "proto/proto.h"
#include "proto/serialization.h"

namespace
{
using namespace Proto;

struct Sample
{
    int    a{ 2 };
    size_t b{ 4 };
    short  c{ 7 };

    std::vector<std::string> v{ "abc", "def" };

    std::optional<std::string> s1{ "ghi" };
    std::optional<std::string> s2{};

    auto tie() { return std::tie(a, b, c, v, s1, s2); }
    auto tie() const { return std::tie(a, b, c, v, s1, s2); }
};

TEST(Deserialization, RestoringClasses)
{
    auto s = Sample{};
    s.a    = 100;

    auto res = serialize(s);

    auto ptr = DeserializationPtr{ res.constBufferSequence };

    auto restoredOpt = deserialize<Sample>(ptr);

    ASSERT_TRUE(restoredOpt.has_value());

    const auto &restored = restoredOpt.value();

    ASSERT_EQ(s.a, restored.a);
    ASSERT_EQ(s.b, restored.b);
    ASSERT_EQ(s.c, restored.c);
    ASSERT_EQ(s.v, restored.v);
    ASSERT_EQ(s.s1, restored.s1);
    ASSERT_EQ(s.s2, restored.s2);
}

}    // namespace
