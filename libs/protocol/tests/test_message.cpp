#include <limits>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "protocol/message.h"

#include "utils/memorychecker.h"

using namespace protocol;

namespace
{

TEST(Message, Initialization)
{
    auto message = Message{};
    ASSERT_FALSE(message.valid());
}

TEST(Message, OneInteger)
{
    auto message = Message{};
    message << 1;

    ASSERT_TRUE(message.valid());
    ASSERT_EQ(message.payloadSize(), sizeof(int));
    ASSERT_EQ(message.size(), sizeof(Message::size_type) + message.payloadSize());
    ASSERT_NE(message.message(), nullptr);
    ASSERT_NE(message.payload(), nullptr);
    ASSERT_EQ(message.message() + sizeof(Message::size_type), message.payload());

    auto checker = MemoryChecker{ message.message() };
    ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), message.payloadSize());
    ASSERT_EQ(checker.checkAndAdvance<int>(), 1);
}

TEST(Message, ThreeIntegrals)
{
    auto message = Message{};

    message << size_t{ 2 } << 5 << short{ 3 };

    ASSERT_TRUE(message.valid());
    ASSERT_EQ(message.payloadSize(), sizeof(size_t) + sizeof(int) + sizeof(short));
    ASSERT_EQ(message.size(), sizeof(Message::size_type) + message.payloadSize());
    ASSERT_NE(message.message(), nullptr);
    ASSERT_NE(message.payload(), nullptr);
    ASSERT_EQ(message.message() + sizeof(Message::size_type), message.payload());

    auto checker = MemoryChecker{ message.message() };

    ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), message.payloadSize());
    ASSERT_EQ(checker.checkAndAdvance<size_t>(), size_t{ 2 });
    ASSERT_EQ(checker.checkAndAdvance<int>(), 5);
    ASSERT_EQ(checker.checkAndAdvance<short>(), short{ 3 });
}

TEST(Message, String)
{
    auto message = Message{};

    const auto s = std::string{ "sample" };

    message << s;

    ASSERT_TRUE(message.valid());
    ASSERT_EQ(message.payloadSize(), sizeof(Message::size_type) + s.size());
    ASSERT_EQ(message.size(), sizeof(Message::size_type) + message.payloadSize());
    ASSERT_NE(message.message(), nullptr);
    ASSERT_NE(message.payload(), nullptr);
    ASSERT_EQ(message.message() + sizeof(Message::size_type), message.payload());

    auto checker = MemoryChecker{ message.message() };

    ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), message.payloadSize());
    ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), s.size());
    for (const auto &c : s)
        ASSERT_EQ(checker.checkAndAdvance<char>(), c);
}

TEST(Message, Reserving)
{
    const auto a = 10;
    const auto b = std::string_view{ "abc" };
    const auto c = short{ 5 };

    auto message = Message{};
    message.reserveSpaceFor(100);

    message << a << b << c;

    ASSERT_TRUE(message.valid());
    ASSERT_EQ(message.payloadSize(),
              sizeof(int) + sizeof(Message::size_type) + b.size() + sizeof(short));
    ASSERT_EQ(message.size(), sizeof(Message::size_type) + message.payloadSize());
    ASSERT_NE(message.message(), nullptr);
    ASSERT_NE(message.payload(), nullptr);
    ASSERT_EQ(message.message() + sizeof(Message::size_type), message.payload());

    auto checker = MemoryChecker{ message.message() };

    ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), message.payloadSize());
    ASSERT_EQ(checker.checkAndAdvance<int>(), a);
    ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), b.size());
    for (const auto &c : b)
        ASSERT_EQ(checker.checkAndAdvance<char>(), c);
    ASSERT_EQ(checker.checkAndAdvance<short>(), c);
}

TEST(Message, invalidation)
{
    for (auto bytesToReserve : { std::numeric_limits<size_t>::max(),
                                 (std::numeric_limits<size_t>::max() / 4) * 3 })
    {
        auto message = Message{};
        message << 1;

        ASSERT_TRUE(message.valid());
        ASSERT_EQ(message.payloadSize(), sizeof(int));
        ASSERT_EQ(message.size(), sizeof(Message::size_type) + message.payloadSize());
        ASSERT_NE(message.message(), nullptr);
        ASSERT_NE(message.payload(), nullptr);
        ASSERT_EQ(message.message() + sizeof(Message::size_type), message.payload());

        {
            auto checker = MemoryChecker{ message.message() };
            ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(),
                      message.payloadSize());
            ASSERT_EQ(checker.checkAndAdvance<int>(), 1);
        }

        message.reserveSpaceFor(bytesToReserve);

        ASSERT_FALSE(message.valid());
        ASSERT_EQ(message.size(), sizeof(Message::size_type));
        ASSERT_EQ(message.payloadSize(), 0);

        {
            auto checker = MemoryChecker{ message.message() };
            ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), 0);
        }

        message << 2;
        ASSERT_FALSE(message.valid());
        ASSERT_EQ(message.size(), sizeof(Message::size_type));
        ASSERT_EQ(message.payloadSize(), 0);

        {
            auto checker = MemoryChecker{ message.message() };
            ASSERT_EQ(checker.checkAndAdvance<Message::size_type>(), 0);
        }

        message << std::string{ "asdasd" };
        ASSERT_FALSE(message.valid());
        ASSERT_EQ(message.size(), sizeof(Message::size_type));
        ASSERT_EQ(message.payloadSize(), 0);
    }
}

}    // namespace
