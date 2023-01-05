#include "proto2/frame.hpp"

#include "deserialization/bufferdeserializer.h"

namespace Proto2::Frame
{

std::optional<EventType> parseEventType(boost::asio::const_buffer frame)
{
    auto deserializer = BufferDeserializer{ frame };

    const auto optEventType = deserializer.deserialize<EventType>();

    if (optEventType.has_value())
    {
        return {};
    }
    else
    {
        // validating EventType value

        const auto eventType = optEventType.value();
        auto       good      = false;

        switch (eventType)
        {
        case EventType::REQUEST:
        case EventType::RESPONSE:
            good = true;
            break;
        }

        if (good)
            return eventType;
        else
            return {};
    }
}

std::optional<RequestFrame> parseRequestFrame(boost::asio::const_buffer frame)
{
    auto deserializer = BufferDeserializer{ frame };

    {
        const auto optEventType = deserializer.deserialize<EventType>();
        if (!optEventType.has_value() || optEventType.value() != EventType::REQUEST)
            return {};
    }

    const auto optRequesetId = deserializer.deserialize<size_t>();
    if (!optRequesetId.has_value())
        return {};
    const auto requestId = optRequesetId.value();

    const auto optOpCode = deserializer.deserialize<OpCode>();
    if (!optOpCode.has_value())
        return {};
    {
        // validate the value of the opCode
        auto good = false;

        switch (optOpCode.value())
        {
        case OpCode::ADD_MESSAGE:
        case OpCode::GET_MESSAGES:
        case OpCode::SECONDARY_NODE_READY:
        case OpCode::PING_PONG:
            good = true;
        }

        if (!good)
            return {};
    }
    const auto opCode = optOpCode.value();

    const auto payload = deserializer.leftover();

    return RequestFrame{ requestId, opCode, payload };
}

std::optional<ResponseFrame> parseResponseFrame(boost::asio::const_buffer frame)
{
    auto deserializer = BufferDeserializer{ frame };

    const auto optEventType = deserializer.deserialize<EventType>();
    if (!optEventType.has_value() || optEventType.value() != EventType::RESPONSE)
        return {};
    const auto eventType = optEventType.value();

    const auto optRequestId = deserializer.deserialize<size_t>();
    if (!optRequestId.has_value())
        return {};
    const auto requestId = optRequestId.value();

    const auto payload = deserializer.leftover();

    return ResponseFrame{ requestId, payload };
}

}    // namespace Proto2::Frame
