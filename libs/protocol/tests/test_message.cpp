#include <gtest/gtest.h>

#include "protocol/message/message.h"

using namespace protocol;

namespace
{

TEST(Message, Initialization)
{
    auto message = Message{};
    ASSERT_FALSE(message.valid());
}

}    // namespace
