#include "socketwrapper/socketwrapper.h"

#include <algorithm>
#include <utility>

using error_code = boost::system::error_code;

namespace Proto2
{
SocketWrapper::FrameContext::FrameContext(
    std::vector<boost::asio::const_buffer>                         seq,
    std::function<void(const boost::system::error_code &, size_t)> callback)
    : seq{ std::move(seq) }, callback{ std::move(callback) }
{
}

std::shared_ptr<SocketWrapper>
    SocketWrapper::create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket)
{
    return std::shared_ptr<SocketWrapper>{ new SocketWrapper{ ioContext, std::move(socket) } };
}

SocketWrapper::~SocketWrapper()
{
    invalidate();
}

void SocketWrapper::run()
{
    readFrameSize();
}

void SocketWrapper::writeFrame(
    std::vector<boost::asio::const_buffer>                         frame,
    std::function<void(const boost::system::error_code &, size_t)> callback)
{
    auto context = FrameContext{ std::move(frame), std::move(callback) };
    boost::asio::post(
        ioContext_,
        [this, context = std::move(context), self = shared_from_this()]() mutable
        {
            pendingFrames_.push(std::move(context));
            writeFrameImpl();
        });
}

SocketWrapper::SocketWrapper(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket)
    : ioContext_{ ioContext }, socket_{ std::move(socket) }
{
}

void SocketWrapper::readFrameSize()
{
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(&incomingFrameSize_, sizeof(size_t)),
        boost::asio::transfer_exactly(sizeof(size_t)),
        [this, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (ec)
            {
                EN_LOGE << "Failed to read frame size, invalidating";
                return invalidate();
            }

            readFrame();
        });
}

void SocketWrapper::readFrame()
{
    allocateBuffer(incomingFrameSize_);

    if (bufferSize_ < incomingFrameSize_)
    {
        EN_LOGE << "Failed to allocate buffer, invalidating";
        return invalidate();
    }

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(buffer_.get(), incomingFrameSize_),
        boost::asio::transfer_exactly(incomingFrameSize_),
        [this, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (ec)
            {
                EN_LOGE << "Failed to read frame, invalidating";
                return invalidate();
            }

            incomingFrame(boost::asio::const_buffer(buffer_.get(), incomingFrameSize_));

            readFrameSize();
        });
}

void SocketWrapper::invalidate()
{
    const auto firstTime = !invalidated_;

    if (firstTime)
    {
        invalidated_ = true;
        socket_.close();
    }

    writeFrameImpl();

    if (firstTime)
        invalidated();
}

boost::asio::io_context &SocketWrapper::ioContext()
{
    return ioContext_;
}

void SocketWrapper::writeFrameImpl()
{
    if (writeInProgress_)
        return;

    if (invalidated_)
    {
        while (!pendingFrames_.empty())
        {
            auto &frame = pendingFrames_.front();
            if (frame.callback != nullptr)
                frame.callback(error_code{ boost::asio::error::operation_aborted }, {});
            pendingFrames_.pop();
        }
        return;
    }

    if (pendingFrames_.empty())
        return;

    writeInProgress_ = true;
    auto context     = std::move(pendingFrames_.front());
    pendingFrames_.pop();

    auto seq      = std::move(context.seq);
    auto callback = std::move(context.callback);

    // add frame size
    {
        outcomingFrameSizeBuffer_ = 0;
        for (auto &buffer : seq)
            outcomingFrameSizeBuffer_ += buffer.size();
        std::reverse(seq.begin(), seq.end());
        seq.push_back(boost::asio::const_buffer(&outcomingFrameSizeBuffer_, sizeof(size_t)));
        std::reverse(seq.begin(), seq.end());
    }

    boost::asio::async_write(
        socket_,
        seq,
        [this, callback = std::move(callback), self = shared_from_this()](const error_code &ec, size_t transferred)
        {
            writeInProgress_ = false;

            if (callback != nullptr)
                callback(ec, transferred);

            if (ec)
            {
                EN_LOGE << "Failed to write frame, invalidating";
                invalidate();
            }
            else
            {
                writeFrameImpl();
            }
        });
}

void SocketWrapper::allocateBuffer(size_t bytes)
{
    if (bufferSize_ > kMaxReusedBufferSize)
    {
        bufferSize_ = 0;
        buffer_     = nullptr;
    }

    if (bufferSize_ >= bytes)
        return;

    buffer_.reset(new (std::nothrow) std::byte[bytes]);

    if (buffer_ != nullptr)
        bufferSize_ = bytes;
}

}    // namespace Proto2
