#include "proto/socketwrapper.h"

#include <utility>

using namespace boost::asio;
using error_code = boost::system::error_code;

namespace Proto
{
std::shared_ptr<SocketWrapper> SocketWrapper::create(boost::asio::ip::tcp::socket socket)
{
    return std::shared_ptr<SocketWrapper>(new SocketWrapper{ std::move(socket) });
}

SocketWrapper::SocketWrapper(boost::asio::ip::tcp::socket socket)
    : socket_{ std::move(socket) }
{
}

void SocketWrapper::run()
{
    readFrameSize();
}

void SocketWrapper::readFrameSize()
{
    async_read(socket_,
               buffer(&frameSizeBuffer_, sizeof(size_t)),
               transfer_exactly(sizeof(size_t)),
               [this, self = shared_from_this()](const error_code &ec, size_t)
               {
                   if (ec)
                   {
                       EN_LOGE << "Failed to read from size, invalidating";
                       return invalidate();
                   }

                   readFrame();
               });
}

void SocketWrapper::readFrame()
{
    frameBuffer_.reset(new (std::nothrow) std::byte[frameSizeBuffer_]);

    if (frameBuffer_ == nullptr)
    {
        EN_LOGE << "Failed to allocate memory for frame, invalidating";
        return invalidate();
    }

    async_read(socket_,
               buffer(frameBuffer_.get(), frameSizeBuffer_),
               transfer_exactly(frameSizeBuffer_),
               [this, self = shared_from_this()](const error_code &ec, size_t)
               {
                   if (ec)
                   {
                       EN_LOGE << "Failed to read frame, invalidating";
                       return invalidate();
                   }

                   incomingBuffer(const_buffer(frameBuffer_.get(), frameSizeBuffer_));

                   readFrameSize();
               });
}

void SocketWrapper::invalidate()
{
    socket_.close();
    invalidated();
}

void SocketWrapper::send(std::shared_ptr<Reflection::SerializationContext> context)
{
    if (context == nullptr)
        return;

    {
        auto size = size_t{};
        for (const auto &buffer : context->constBufferSequence())
            size += buffer.size();

        auto sharedSize = std::make_shared<size_t>(size);

        async_write(
            socket_,
            const_buffer(sharedSize.get(), sizeof(size_t)),
            [sharedSize, self = shared_from_this()](const error_code &, size_t) {});
    }

    async_write(socket_,
                context->constBufferSequence(),
                [context, self = shared_from_this()](const error_code &, size_t) {});
}

}    // namespace Proto
