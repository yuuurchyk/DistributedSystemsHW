#include <gtest/gtest.h>

#include "protocol/framebuilder.h"
#include "protocol/request/getstrings.h"

using namespace protocol;

namespace
{
TEST(RequestGetStrings, ValidFormulation)
{
    auto frame =
        FrameBuilder{ codes::Event::REQUEST, codes::OpCode::GET_STRINGS, 1 }.build();
    ASSERT_TRUE(frame.valid());

    auto request = request::Request{ std::move(frame) };
    ASSERT_TRUE(request.valid());

    auto pushStringRequest = request::GetStrings{ std::move(request) };
    ASSERT_TRUE(pushStringRequest.valid());
}

TEST(RequestGetStrings, InvalidFormulation)
{
    {
        const auto request = request::GetStrings{ request::Request{
            FrameBuilder{ codes::Event::REQUEST, codes::OpCode::PUSH_STRING, 2 }
                .build() } };
        ASSERT_FALSE(request.valid());
    }
    {
        const auto request = request::GetStrings{ request::Request{
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::GET_STRINGS, 2 }
                .build() } };
        ASSERT_FALSE(request.valid());
    }
    {
        const auto request = request::GetStrings{ request::Request{
            FrameBuilder{ codes::Event::REQUEST, codes::OpCode::GET_STRINGS, 2 }
                .addToBody(1)
                .build() } };
        ASSERT_FALSE(request.valid());
    }
}

}    // namespace
