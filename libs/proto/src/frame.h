#pragma once

#include "proto/proto.h"

/**
 * Request Frame:
 * EventType::REQUEST
 * size_t               requestId
 * OpCode               opCode
 * Variable body
 *
 * Response Frame:
 * EventType::RESPONSE
 * size_t               requestId
 * Variable body
 */
namespace Proto
{

struct RequestHeader
{
    static constexpr Proto::EventType kEventType{ Proto::EventType::REQUEST };
    size_t                            requestId;
    Proto::OpCode                     opCode;

    auto tie() const { return std::tie(kEventType, requestId, opCode); }
};
struct ResponseHeader
{
    static constexpr Proto::EventType kEventType{ Proto::EventType::RESPONSE };
    size_t                            requestId;

    auto tie() const { return std::tie(kEventType, requestId); }
};

struct IncomingHeader
{
    Proto::EventType eventType;
    size_t           requestId;

    auto tie() { return std::tie(eventType, requestId); }
};

}    // namespace Proto
