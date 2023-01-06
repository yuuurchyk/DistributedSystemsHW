#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "utils/copymove.h"

#include "codes/codes.h"

namespace Proto2::Request
{
class AbstractRequest : public std::enable_shared_from_this<AbstractRequest>
{
    DISABLE_COPY_MOVE(AbstractRequest)
public:
    AbstractRequest()          = default;
    virtual ~AbstractRequest() = default;

    virtual const OpCode &opCode() const = 0;

    /**
     * @brief serializes request payload
     *
     * @note all entries present in resulting const buffer sequence
     * should live as long as the request is alive
     * @note this function should only serialize the request payload
     */
    virtual void serializePayload(std::vector<boost::asio::const_buffer> &constBufferSeq) const = 0;

    // note: all implementations of request should also have
    // static std::shared_ptr<RequestClass> fromPayload(boost::asio::const_buffer)
};

}    // namespace Proto2::Request
