#pragma once

#include "protocol/frame.h"

namespace protocol::request
{

class Request : public Frame
{
public:
    Request(Frame frame);
};

}    // namespace protocol::request
