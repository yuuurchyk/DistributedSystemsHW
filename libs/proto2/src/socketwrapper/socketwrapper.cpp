#include "socketwrapper/socketwrapper.h"

#include <algorithm>
#include <utility>

using error_code = boost::system::error_code;

namespace Proto2
{
std::shared_ptr<SocketWrapper>
    SocketWrapper::create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket)
{
    return std::shared_ptr<SocketWrapper>{ new SocketWrapper{ ioContext, std::move(socket) } };
}

SocketWrapper::~SocketWrapper()
{
    invalidate();
}

void SocketWrapper::writeFrame(std::vector<boost::asio::const_buffer> frame, WriteFrameCallback callback)
{
    auto pendingWrite = PendingWrite{ std::move(frame), std::move(callback) };
    boost::asio::post(
        [this, pendingWrite = std::move(pendingWrite), self = shared_from_this()]() mutable
        {
            pendingWrites_.push(std::move(pendingWrite));
            checkPendingWrites();
        });
}

void SocketWrapper::run()
{
    readFrameSize();
}

void SocketWrapper::invalidate()
{
    const auto firstTime = !invalidated_;

    if (firstTime)
    {
        invalidated_ = true;
        socket_.close();
    }

    checkPendingWrites();

    if (firstTime)
        invalidated();
}

SocketWrapper::PendingWrite::PendingWrite(std::vector<boost::asio::const_buffer> frame, WriteFrameCallback callback)
    : frame{ std::move(frame) }, callback{ std::move(callback) }
{
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
            else
            {
                readFrame();
            }
        });
}

void SocketWrapper::readFrame()
{
    if (incomingFrameSize_ > incomingFrameBuffer_.size() || incomingFrameBuffer_.size() > kIncomingBufferMaxReuseSize)
        incomingFrameBuffer_.resize(incomingFrameSize_);

    if (incomingFrameBuffer_.size() < incomingFrameSize_)
    {
        EN_LOGE << "Failed to allocate read buffer, invalidating";
        return invalidate();
    }

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(incomingFrameBuffer_.get(), incomingFrameSize_),
        boost::asio::transfer_exactly(incomingFrameSize_),
        [this, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (ec)
            {
                EN_LOGE << "Failed to read incoming frame, invalidating";
                return invalidate();
            }
            else
            {
                incomingFrame(boost::asio::const_buffer(incomingFrameBuffer_.get(), incomingFrameSize_));
                readFrameSize();
            }
        });
}

void SocketWrapper::checkPendingWrites()
{
    if (writeInProgress_)
        return;

    if (invalidated_)
    {
        while (!pendingWrites_.empty())
        {
            auto &pendingWrite = pendingWrites_.front();
            pendingWrite.callback(error_code{ boost::asio::error::operation_aborted }, {});
            pendingWrites_.pop();
        }
        return;
    }

    if (pendingWrites_.empty())
        return;

    writeInProgress_ = true;

    auto frame    = std::move(pendingWrites_.front().frame);
    auto callback = std::move(pendingWrites_.front().callback);

    pendingWrites_.pop();

    {
        // calculate and add frame size
        outcomingFrameSize_ = 0;
        for (const auto &piece : frame)
            outcomingFrameSize_ += piece.size();

        // add frame size buffer to the beginning of frame
        std::reverse(frame.begin(), frame.end());
        frame.push_back(boost::asio::const_buffer{ &outcomingFrameSize_, sizeof(size_t) });
        std::reverse(frame.begin(), frame.end());
    }

    boost::asio::async_write(
        socket_,
        std::move(frame),
        [this, callback = std::move(callback), self = shared_from_this()](const error_code &ec, size_t transferred)
        {
            callback(ec, transferred);

            if (ec)
            {
                EN_LOGE << "Failed to write frame, invalidating";
                invalidate();
            }

            writeInProgress_ = false;
            checkPendingWrites();
        });
}

}    // namespace Proto2
