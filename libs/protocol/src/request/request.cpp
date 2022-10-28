#include "protocol/request/request.h"

#include <utility>

namespace protocol::request
{

Request::Request(Frame &&frame) : Frame{ std::move(frame) }
{
    decideOnValidity();
}

void Request::decideOnValidity()
{
    if (!valid())
        return;

    if (event() != codes::Event::REQUEST)
        return invalidate();
}

}    // namespace protocol::request
