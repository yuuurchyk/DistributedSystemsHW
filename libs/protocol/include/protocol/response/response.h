#pragma once

#include "protocol/frame.h"
#include "protocol/framebuilder.h"
#include "protocol/request/request.h"

namespace protocol::response
{
class Response : public Frame
{
public:
    Response(Frame &&frame);

protected:
    static FrameBuilder responseFrameBuilder(const request::Request &);
    static FrameBuilder responseFrameBuilder(const request::Request &, size_t bodySize);

private:
    void decideOnValidity();
};

}    // namespace protocol::response
