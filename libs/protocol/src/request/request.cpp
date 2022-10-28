#include "protocol/request/request.h"

#include <utility>

namespace protocol::request
{

Request::Request(Frame frame) : Frame{ std::move(frame) }
{
    if (!valid())
        return;
    if (event() != codes::Event::REQUEST)
        invalidate();
}

}    // namespace protocol::request
