#include "protocol/response/response.h"

#include <utility>

#include "protocol/codes.h"

namespace protocol::response
{
FrameBuilder Response::responseFrameBuilder(const request::Request &request)
{
    return responseFrameBuilder(request, 0);
}

FrameBuilder Response::responseFrameBuilder(const request::Request &request,
                                            size_t                  bodySize)
{
    return FrameBuilder{
        codes::Event::RESPONSE, request.opCode(), request.requestId(), bodySize
    };
}

Response::Response(Frame &&frame) : Frame{ std::move(frame) }
{
    decideOnValidity();
}

void Response::decideOnValidity()
{
    if (!valid())
        return;

    if (event() != codes::Event::RESPONSE)
        return invalidate();
}

}    // namespace protocol::response
