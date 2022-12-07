#include <gtest/gtest.h>

#include "reflection/deserialization.h"
#include "reflection/serialization.h"

namespace
{
struct Sample
{
    int    a{ 2 };
    size_t b{ 4 };
    short  c{ 7 };

    std::vector<std::string> v{ "abc", "def" };

    std::string s1{ "ghi" };
    std::string s2{};

    auto tie() { return std::tie(a, b, c, v, s1, s2); }
    auto tie() const { return std::tie(a, b, c, v, s1, s2); }
};

TEST(Deserialization, RestoringClasses)
{
    auto s = std::make_shared<Sample>();
    s->a   = 100;

    auto serializationContext = Reflection::SerializationContext{};
    serializationContext.serializeAndHold(s);

    auto deserializationContext =
        Reflection::DeserializationContext{ serializationContext.constBufferSequence() };
    auto optRestored = deserializationContext.deserialize<Sample>();

    ASSERT_TRUE(optRestored.has_value());

    const auto &a = *s;
    const auto &b = optRestored.value();

    ASSERT_EQ(a.a, b.a);
    ASSERT_EQ(a.b, b.b);
    ASSERT_EQ(a.c, b.c);
    ASSERT_EQ(a.v, b.v);
    ASSERT_EQ(a.s1, b.s1);
    ASSERT_EQ(a.s2, b.s2);
}

}    // namespace
