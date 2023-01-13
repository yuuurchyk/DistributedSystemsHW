#include "frame.h"

#include "deserialization/bufferdeserializer.h"

namespace Proto::Frame
{
std::optional<EventType> parseEventType(boost::asio::const_buffer frame)
{
    auto deserializer = BufferDeserializer{ frame };

    const auto optEventType = deserializer.deserialize<EventType>();

    if (optEventType.has_value())
    {
        // validating EventType value

        const auto eventType = optEventType.value();
        auto       good      = false;

        switch (eventType)
        {
        case EventType::REQUEST:    // fall-through
        case EventType::RESPONSE:
            good = true;
            break;
        }

        if (good)
            return eventType;
        else
            return {};
    }
    else
    {
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
        case OpCode::ADD_MESSAGE:             // fall-through
        case OpCode::GET_MESSAGES:            // fall-through
        case OpCode::SECONDARY_NODE_READY:    // fall-through
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

std::vector<boost::asio::const_buffer> constructRequestHeaderWoOwnership(const size_t &requestId, const OpCode &opCode)
{
    static constexpr EventType kEventType{ EventType::REQUEST };

    auto res = std::vector<boost::asio::const_buffer>{};
    res.reserve(3);

    res.push_back(boost::asio::const_buffer{ &kEventType, sizeof(EventType) });
    res.push_back(boost::asio::const_buffer{ &requestId, sizeof(size_t) });
    res.push_back(boost::asio::const_buffer{ &opCode, sizeof(OpCode) });

    return res;
}

std::vector<boost::asio::const_buffer> constructResponseHeaderWoOwnership(const size_t &responseId)
{
    static constexpr EventType kEventType{ EventType::RESPONSE };

    auto res = std::vector<boost::asio::const_buffer>{};
    res.reserve(2);

    res.push_back(boost::asio::const_buffer{ &kEventType, sizeof(EventType) });
    res.push_back(boost::asio::const_buffer{ &responseId, sizeof(size_t) });

    return res;
}

}    // namespace Proto::Frame
