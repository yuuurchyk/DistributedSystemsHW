#include "socketwrapper/socketwrapper.h"

#include <algorithm>
#include <utility>

using error_code = boost::system::error_code;

namespace
{
const error_code kAbortedErrorCode{ boost::asio::error::operation_aborted };

}    // namespace

namespace Proto2
{
std::shared_ptr<SocketWrapper> SocketWrapper::create(
    std::string                  id,
    boost::asio::io_context     &executionContext,
    boost::asio::ip::tcp::socket socket)
{
    return std::shared_ptr<SocketWrapper>{ new SocketWrapper{ std::move(id), executionContext, std::move(socket) } };
}

SocketWrapper::~SocketWrapper()
{
    // don't emit signal in dtor

    if (!wasInvalidated())
    {
        invalidated_ = true;
        socket_.close();
    }

    checkPendingWrites();
}

void SocketWrapper::invalidate()
{
    const auto firstTime = !wasInvalidated();

    if (firstTime)
    {
        invalidated_ = true;
        socket_.close();
    }

    checkPendingWrites();

    if (firstTime)
        invalidated();
}

bool SocketWrapper::wasInvalidated() const
{
    return invalidated_;
}

void SocketWrapper::writeFrame(std::vector<boost::asio::const_buffer> frame, WriteFrameCallback_fn callback)
{
    auto pendingWrite = PendingWrite{ std::move(frame), std::move(callback) };
    boost::asio::post(
        executionContext_,
        [this, pendingWrite = std::move(pendingWrite), weakSelf = weak_from_this()]() mutable
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
            {
                pendingWrite.callback(kAbortedErrorCode, {});
            }
            else
            {
                pendingWrites_.push(std::move(pendingWrite));
                checkPendingWrites();
            }
        });
}

void SocketWrapper::run()
{
    readFrameSize();
}

boost::asio::io_context &SocketWrapper::executionContext()
{
    return executionContext_;
}

SocketWrapper::PendingWrite::PendingWrite(std::vector<boost::asio::const_buffer> frame, WriteFrameCallback_fn callback)
    : frame{ std::move(frame) }, callback{ std::move(callback) }
{
}

SocketWrapper::SocketWrapper(
    std::string                  id,
    boost::asio::io_context     &executionContext,
    boost::asio::ip::tcp::socket socket)
    : logger::StringIdEntity<SocketWrapper>{ std::move(id) },
      executionContext_{ executionContext },
      socket_{ std::move(socket) }
{
}

void SocketWrapper::readFrameSize()
{
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(&incomingFrameSize_, sizeof(size_t)),
        boost::asio::transfer_exactly(sizeof(size_t)),
        [this, weakSelf = weak_from_this()](const error_code &ec, size_t)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (ec)
            {
                EN_LOGE << "Failed to read frame size, invalidating";
                invalidate();
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
        [this, weakSelf = weak_from_this()](const error_code &ec, size_t)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (ec)
            {
                EN_LOGE << "Failed to read incoming frame, invalidating";
                invalidate();
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
    if (wasInvalidated())
    {
        while (!pendingWrites_.empty())
        {
            auto &pendingWrite = pendingWrites_.front();
            pendingWrite.callback(kAbortedErrorCode, {});
            pendingWrites_.pop();
        }
        return;
    }

    if (writeInProgress_)
        return;

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
        [this, callback = std::move(callback), weakSelf = weak_from_this()](const error_code &ec, size_t transferred)
        {
            const auto self = weakSelf.lock();

            callback(ec, transferred);

            if (self == nullptr)
                return;

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
