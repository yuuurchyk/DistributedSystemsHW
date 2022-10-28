#include <string_view>

#include <gtest/gtest.h>

#include "protocol/codes.h"
#include "protocol/frame.h"
#include "protocol/framebuilder.h"
#include "utils/memorychecker.h"

using namespace protocol;

namespace
{

TEST(Frame, ValidCreation)
{
    const auto frame =
        FrameBuilder{ codes::Event::REQUEST, codes::OpCode::PUSH_STRING, 1 }
            .addToBody(1)
            .addToBody(std::string_view{ "sample" })
            .build();

    ASSERT_TRUE(frame.valid());
    ASSERT_EQ(frame.event(), codes::Event::REQUEST);
    ASSERT_EQ(frame.opCode(), codes::OpCode::PUSH_STRING);
    ASSERT_EQ(frame.requestId(), 1);

    const auto body = frame.body();
    ASSERT_EQ(body.size(), sizeof(int) + 6);
}

TEST(Frame, InvalidCreation)
{
    {
        const auto frame =
            FrameBuilder{ codes::Event::ERROR, codes::OpCode::PUSH_STRING, 1 }.build();
        ASSERT_FALSE(frame.valid());
    }
    {
        const auto frame =
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::ERROR, 1 }.build();
        ASSERT_FALSE(frame.valid());
    }
}

TEST(Frame, MemoryInspection)
{
    auto frame = FrameBuilder{ codes::Event::REQUEST, codes::OpCode::GET_STRINGS, 5 }
                     .addToBody(1)
                     .addToBody(std::string_view{ "abc" })
                     .addToBody(5)
                     .build();

    ASSERT_TRUE(frame.valid());

    const auto buffer = frame.flushBuffer();
    ASSERT_FALSE(buffer.invalidated());

    const auto expectedSize = sizeof(Frame::size_type) + sizeof(codes::Event) +
                              sizeof(codes::OpCode) + sizeof(Frame::size_type) +
                              sizeof(int) + 3 + sizeof(int);
    ASSERT_EQ(buffer.size(), expectedSize);

    auto checker = MemoryChecker{ buffer.data() };
    ASSERT_EQ(checker.checkAndAdvance<Frame::size_type>(), expectedSize);
    ASSERT_EQ(checker.checkAndAdvance<codes::Event>(), codes::Event::REQUEST);
    ASSERT_EQ(checker.checkAndAdvance<codes::OpCode>(), codes::OpCode::GET_STRINGS);
    ASSERT_EQ(checker.checkAndAdvance<Frame::size_type>(), 5);
    ASSERT_EQ(checker.checkAndAdvance<int>(), 1);
    ASSERT_EQ(checker.checkAndAdvance<char>(), 'a');
    ASSERT_EQ(checker.checkAndAdvance<char>(), 'b');
    ASSERT_EQ(checker.checkAndAdvance<char>(), 'c');
    ASSERT_EQ(checker.checkAndAdvance<int>(), 5);
}

}    // namespace
