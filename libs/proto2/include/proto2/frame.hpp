#pragma once

#include <optional>

#include <boost/asio.hpp>

#include "proto2/codes.h"

/**
 * The structure of request frame:
 * EventType::REQUEST
 * size_t requestId
 * OpCode
 * Variable body
 *
 * The structure of response frame:
 * EventType::RESPONSE
 * size_t requestId
 * Variable body
 */
namespace Proto2::Frame
{
std::optional<EventType> parseEventType(boost::asio::const_buffer frame);

struct RequestFrame
{
    size_t                    requestId{};
    OpCode                    opCode{};
    boost::asio::const_buffer payload{};
};
std::optional<RequestFrame> parseRequestFrame(boost::asio::const_buffer frame);

struct ResponseFrame
{
    size_t                    requestId{};
    boost::asio::const_buffer payload{};
};
std::optional<ResponseFrame> parseResponseFrame(boost::asio::const_buffer response);

}    // namespace Proto2::Frame
