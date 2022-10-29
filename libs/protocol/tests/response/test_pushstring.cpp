#include <gtest/gtest.h>

#include "protocol/framebuilder.h"
#include "protocol/response/pushstring.h"

using namespace protocol;

namespace
{

TEST(ResponsePushString, ValidFormulation)
{
    {
        auto frame =
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::PUSH_STRING, 1 }.build();
        ASSERT_TRUE(frame.valid());

        auto response = response::Response{ std::move(frame) };
        ASSERT_TRUE(response.valid());

        auto getStringsResponse = response::PushString{ std::move(response) };
        ASSERT_TRUE(getStringsResponse.valid());
    }
}

TEST(ResponsePushString, InvalidFormulation)
{
    {
        auto frame = FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::GET_STRINGS, 1 }
                         .addToBody(1)
                         .build();
        ASSERT_TRUE(frame.valid());

        auto response = response::Response{ std::move(frame) };
        ASSERT_TRUE(response.valid());

        auto getStringsResponse = response::PushString{ std::move(response) };
        ASSERT_FALSE(getStringsResponse.valid());
    }
}

}    // namespace
