#include <gtest/gtest.h>

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

    auto tie() const { return std::tie(a, b, c, v, s1, s2); }
};

TEST(Serialization, CustomClass)
{
    static_assert(Reflection::HasSerializableTie<Sample>);
    auto val = std::make_shared<Sample>();

    auto context = Reflection::SerializationContext{};
    context.serializeAndHold(val);

    ASSERT_EQ(context.constBufferSequence().size(), 12);
}

}    // namespace
