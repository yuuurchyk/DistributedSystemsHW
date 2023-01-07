#pragma once

#include <optional>
#include <vector>

#include <boost/asio.hpp>

#include "codes/codes.h"

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

/**
 * @note make sure that passed @p requestId and @p opCode live
 * long enough for the buffer to remain valid
 *
 * @note payload should be added separately
 */
std::vector<boost::asio::const_buffer> constructRequestHeaderWoOwnership(const size_t &requestId, const OpCode &opCode);

struct ResponseFrame
{
    size_t                    requestId{};
    boost::asio::const_buffer payload{};
};
std::optional<ResponseFrame> parseResponseFrame(boost::asio::const_buffer frame);

/**
 * @note make sure that passed @p responseId live
 * long enough for the buffer to remain valid
 *
 * @note payload should be added separately
 */
std::vector<boost::asio::const_buffer> constructResponseHeaderWoOwnership(const size_t &responseId);

}    // namespace Proto2::Frame
