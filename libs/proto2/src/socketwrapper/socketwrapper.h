#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "proto2/signal.h"

#include "socketwrapper/buffer.h"

namespace Proto2
{
class SocketWrapper : public std::enable_shared_from_this<SocketWrapper>, private logger::Entity<SocketWrapper>
{
    DISABLE_COPY_MOVE(SocketWrapper)
public:
    // expected io context that is associated with the socket
    [[nodiscard]] static std::shared_ptr<SocketWrapper> create(boost::asio::io_context &, boost::asio::ip::tcp::socket);
    ~SocketWrapper();

    using WriteFrameCallback_fn = std::function<void(const boost::system::error_code &, size_t)>;

    // note: thread safe
    // expected callback capture the data, that is referenced in frame for the buffers to remain valid
    void writeFrame(std::vector<boost::asio::const_buffer> frame, WriteFrameCallback_fn callback);

    void run();
    void invalidate();

    boost::asio::io_context &ioContext();

public:    // signals
    signal<boost::asio::const_buffer> incomingFrame;
    signal<>                          invalidated;

private:
    struct PendingWrite
    {
        DISABLE_COPY_DEFAULT_MOVE(PendingWrite);

        PendingWrite(std::vector<boost::asio::const_buffer> frame, WriteFrameCallback_fn callback);

        std::vector<boost::asio::const_buffer> frame;
        WriteFrameCallback_fn                  callback;
    };

    SocketWrapper(boost::asio::io_context &, boost::asio::ip::tcp::socket);

    void readFrameSize();
    void readFrame();

    void checkPendingWrites();

private:
    boost::asio::io_context     &ioContext_;
    boost::asio::ip::tcp::socket socket_;

    bool invalidated_{};

    static constexpr size_t kIncomingBufferMaxReuseSize{ 128 };
    size_t                  incomingFrameSize_;
    Buffer                  incomingFrameBuffer_;

    bool                     writeInProgress_{};
    size_t                   outcomingFrameSize_;
    std::queue<PendingWrite> pendingWrites_;
};

}    // namespace Proto2
