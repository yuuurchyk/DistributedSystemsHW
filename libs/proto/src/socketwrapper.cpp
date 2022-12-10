#include "socketwrapper.h"

#include <algorithm>
#include <utility>

using namespace boost::asio;
using error_code = boost::system::error_code;

namespace Proto
{
std::shared_ptr<SocketWrapper>
    SocketWrapper::create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket)
{
    return std::shared_ptr<SocketWrapper>(new SocketWrapper{ ioContext, std::move(socket) });
}

SocketWrapper::SocketWrapper(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket)
    : socket_{ std::move(socket) }, ioContext_{ ioContext }
{
}

void SocketWrapper::run()
{
    readFrameSize();
}

void SocketWrapper::readFrameSize()
{
    async_read(
        socket_,
        buffer(&frameSize_, sizeof(size_t)),
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
    if (bufferSize_ < frameSize_ || bufferSize_ == 0)
    {
        bufferSize_ = std::max<size_t>(1, frameSize_);
        buffer_.reset(new (std::nothrow) std::byte[bufferSize_]);

        if (buffer_ == nullptr)
        {
            bufferSize_ = 0;
            EN_LOGE << "Failed to allocate memory for frame, invalidating";
            return invalidate();
        }
    }

    async_read(
        socket_,
        buffer(buffer_.get(), frameSize_),
        transfer_exactly(frameSize_),
        [this, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (ec)
            {
                EN_LOGE << "Failed to read frame, invalidating";
                return invalidate();
            }

            incomingBuffer(const_buffer(buffer_.get(), frameSize_));

            readFrameSize();
        });
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

    async_write(
        socket_, context->constBufferSequence(), [context, self = shared_from_this()](const error_code &, size_t) {});
}

void SocketWrapper::invalidate()
{
    socket_.close();
    invalidated();
}

boost::asio::io_context &SocketWrapper::ioContext()
{
    return ioContext_;
}

}    // namespace Proto
