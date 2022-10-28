#include <limits>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "protocol/buffer.h"

#include "utils/memorychecker.h"

using namespace protocol;

namespace
{

TEST(Buffer, Initialization)
{
    auto buffer = Buffer{};
    ASSERT_FALSE(buffer.invalidated());
}

TEST(Buffer, OneInteger)
{
    auto buffer = Buffer{};
    buffer << 1;

    ASSERT_FALSE(buffer.invalidated());
    ASSERT_EQ(buffer.size(), sizeof(int));
    ASSERT_NE(buffer.data(), nullptr);

    auto checker = MemoryChecker{ buffer.data() };
    ASSERT_EQ(checker.checkAndAdvance<int>(), 1);
}

TEST(Buffer, ThreeIntegrals)
{
    auto buffer = Buffer{};

    const auto a = size_t{ 2 };
    const auto b = 5;
    const auto c = short{ 3 };

    buffer << a << b << c;

    ASSERT_FALSE(buffer.invalidated());
    ASSERT_EQ(buffer.size(), sizeof(a) + sizeof(b) + sizeof(c));
    ASSERT_NE(buffer.data(), nullptr);

    auto checker = MemoryChecker{ buffer.data() };
    ASSERT_EQ(checker.checkAndAdvance<size_t>(), a);
    ASSERT_EQ(checker.checkAndAdvance<int>(), b);
    ASSERT_EQ(checker.checkAndAdvance<short>(), c);
}

TEST(Buffer, String)
{
    auto buffer = Buffer{};

    const auto s = std::string{ "sample" };

    buffer << s;

    ASSERT_FALSE(buffer.invalidated());
    ASSERT_EQ(buffer.size(), s.size());
    ASSERT_NE(buffer.data(), nullptr);

    auto checker = MemoryChecker{ buffer.data() };

    for (const auto &c : s)
        ASSERT_EQ(checker.checkAndAdvance<char>(), c);
}

TEST(Buffer, Reserving)
{
    const auto a = 10;
    const auto b = std::string_view{ "abc" };
    const auto c = short{ 5 };

    auto buffer = Buffer{};
    buffer.reserveSpaceFor(100);
    const auto previousData = buffer.data();

    buffer << a << b << c;
    const auto currentData = buffer.data();

    ASSERT_EQ(previousData, currentData);

    ASSERT_FALSE(buffer.invalidated());
    ASSERT_EQ(buffer.size(), sizeof(a) + b.size() + sizeof(c));
    ASSERT_NE(buffer.data(), nullptr);

    auto checker = MemoryChecker{ buffer.data() };

    ASSERT_EQ(checker.checkAndAdvance<int>(), a);
    for (const auto &character : b)
        ASSERT_EQ(checker.checkAndAdvance<char>(), character);
    ASSERT_EQ(checker.checkAndAdvance<short>(), c);
}

TEST(Buffer, Invalidation)
{
    for (auto bytesToReserve : { std::numeric_limits<size_t>::max(),
                                 (std::numeric_limits<size_t>::max() / 4) * 3 })
    {
        auto       buffer = Buffer{};
        const auto a      = 1;
        buffer << a;

        ASSERT_FALSE(buffer.invalidated());
        ASSERT_EQ(buffer.size(), sizeof(int));
        ASSERT_NE(buffer.data(), nullptr);

        auto checker = MemoryChecker{ buffer.data() };
        ASSERT_EQ(checker.checkAndAdvance<int>(), a);

        buffer.reserveSpaceFor(bytesToReserve);

        ASSERT_TRUE(buffer.invalidated());
        ASSERT_EQ(buffer.size(), 0);
        ASSERT_EQ(buffer.data(), nullptr);

        buffer << 2;

        ASSERT_TRUE(buffer.invalidated());
        ASSERT_EQ(buffer.size(), 0);
        ASSERT_EQ(buffer.data(), nullptr);

        buffer << std::string{ "sample" };

        ASSERT_TRUE(buffer.invalidated());
        ASSERT_EQ(buffer.size(), 0);
        ASSERT_EQ(buffer.data(), nullptr);
    }
}

}    // namespace
