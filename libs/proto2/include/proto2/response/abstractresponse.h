#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "utils/copymove.h"

#include "proto2/codes.h"

namespace Proto2::Response
{

class AbstractResponse : public std::enable_shared_from_this<AbstractResponse>
{
    DISABLE_COPY_MOVE(AbstractResponse)

public:
    AbstractResponse()          = default;
    virtual ~AbstractResponse() = default;

    virtual const OpCode &opCode() const = 0;

    /**
     * @brief serializes response payload
     *
     * @note all entries present in resulting const buffer sequence
     * should live as long as the request is alive
     * @note this function should only serialize the request payload
     */
    virtual void serializePayload(std::vector<boost::asio::const_buffer> &constBufferSeq) const = 0;

    // note: all implementations of response should also have
    // static std::unique_ptr<RequestClass> fromPayload(boost::asio::const_buffer)
};

}    // namespace Proto2::Response
