#include <gtest/gtest.h>

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

TEST(Serialization, CustomClass)
{
    const auto &val = Sample{};

    const auto res = serialize(val);
    ASSERT_EQ(res.constBufferSequence.size(), 12);
}

}    // namespace
