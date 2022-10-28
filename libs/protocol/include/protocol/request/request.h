#pragma once

#include "protocol/frame.h"

namespace protocol::request
{

class Request : public Frame
{
public:
    Request(Frame &&frame);

private:
    void decideOnValidity();
};

}    // namespace protocol::request
