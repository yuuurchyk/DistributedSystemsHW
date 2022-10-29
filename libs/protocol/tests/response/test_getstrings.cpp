#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "protocol/framebuilder.h"
#include "protocol/response/getstrings.h"

using namespace protocol;

namespace
{

TEST(ResponseGetStrings, ValidFormulation)
{
    {
        auto frame =
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::GET_STRINGS, 1 }.build();
        ASSERT_TRUE(frame.valid());

        auto response = response::Response{ std::move(frame) };
        ASSERT_TRUE(response.valid());

        auto getStringsResponse = response::GetStrings{ std::move(response) };
        ASSERT_TRUE(response.valid());
        ASSERT_TRUE(getStringsResponse.strings().empty());
    }
    {
        const auto response = response::GetStrings{ response::Response{
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::GET_STRINGS, 1 }
                .addToBody(size_t{ 5 })
                .addToBody(std::string_view{ "abcde" })
                .addToBody(size_t{ 1 })
                .addToBody(std::string_view{ "d" })
                .addToBody(size_t{})
                .build() } };
        ASSERT_TRUE(response.valid());

        ASSERT_EQ(response.strings().size(), 3);
        ASSERT_EQ(response.strings()[0].size(), 5);
        ASSERT_EQ(response.strings()[0], std::string_view{ "abcde" });
        ASSERT_EQ(response.strings()[1].size(), 1);
        ASSERT_EQ(response.strings()[1], std::string_view{ "d" });
        ASSERT_EQ(response.strings()[2].size(), 0);
    }
    {
        const auto request  = request::GetStrings{ request::Request{
            FrameBuilder{ codes::Event::REQUEST, codes::OpCode::GET_STRINGS, 2 }
                .build() } };
        const auto response = response::GetStrings::answer(request, { "a", "bc" });

        ASSERT_TRUE(request.valid());
        ASSERT_TRUE(response.valid());
        ASSERT_EQ(request.opCode(), response.opCode());
        ASSERT_EQ(request.requestId(), response.requestId());

        ASSERT_EQ(response.strings().size(), 2);
        ASSERT_EQ(response.strings()[0], "a");
    }
}

TEST(ResponseGetStrings, InvalidFormulation)
{
    {
        const auto response = response::GetStrings{ response::Response{
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::GET_STRINGS, 1 }
                .addToBody(size_t{ 5 })
                .addToBody(std::string_view{ "abcd" })
                .build() } };
        ASSERT_FALSE(response.valid());
    }
    {
        const auto response = response::GetStrings{ response::Response{
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::GET_STRINGS, 1 }
                .addToBody(size_t{ 3 })
                .addToBody(std::string_view{ "abcd" })
                .build() } };
        ASSERT_FALSE(response.valid());
    }
    {
        const auto response = response::GetStrings{ response::Response{
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::GET_STRINGS, 1 }
                .addToBody(size_t{ 3 })
                .build() } };
        ASSERT_FALSE(response.valid());
    }
    {
        auto frame =
            FrameBuilder{ codes::Event::RESPONSE, codes::OpCode::PUSH_STRING, 1 }.build();
        ASSERT_TRUE(frame.valid());

        auto response = response::Response{ std::move(frame) };
        ASSERT_TRUE(response.valid());

        auto getStringsResponse = response::GetStrings{ std::move(response) };
        ASSERT_FALSE(getStringsResponse.valid());
        ASSERT_TRUE(getStringsResponse.strings().empty());
    }
}

}    // namespace
