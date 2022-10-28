#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "protocol/framebuilder.h"
#include "protocol/request/pushstring.h"

using namespace protocol;

namespace
{
TEST(RequestPushString, ValidFormulation)
{
    {
        auto frame = FrameBuilder{ codes::Event::REQUEST, codes::OpCode::PUSH_STRING, 1 }
                         .addToBody(std::string_view{ "abc" })
                         .build();
        ASSERT_TRUE(frame.valid());

        auto request = request::Request{ std::move(frame) };
        ASSERT_TRUE(request.valid());

        auto pushStringRequest = request::PushString{ std::move(request) };
        ASSERT_TRUE(pushStringRequest.valid());
        ASSERT_EQ(std::string{ pushStringRequest.string() }, "abc");
    }
    {
        const auto request = request::PushString{ request::Request{
            FrameBuilder{ codes::Event::REQUEST, codes::OpCode::PUSH_STRING, 2 }
                .build() } };
        ASSERT_TRUE(request.valid());
        ASSERT_EQ(request.string().size(), 0);
    }
}

TEST(RequestPushString, InvalidFormulation)
{
    {
        const auto request = request::PushString{ request::Request{
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::PUSH_STRING, 2 }
                .build() } };
        ASSERT_FALSE(request.valid());
    }
    {
        const auto request = request::PushString{ request::Request{
            FrameBuilder{ codes::Event::REQUEST, codes::OpCode::ERROR, 2 }.build() } };
        ASSERT_FALSE(request.valid());
    }
}

}    // namespace
