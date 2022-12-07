#pragma once

#include "proto/socketwrapper.h"

namespace Proto
{
template <Reflection::Serializable T>
void SocketWrapper::send(std::shared_ptr<const T> ptr)
{
    if (ptr == nullptr)
        return;

    auto context = std::make_shared<Reflection::SerializationContext>();
    context->serializeAndHold(ptr);

    send(std::move(context));
}

}    // namespace Proto
