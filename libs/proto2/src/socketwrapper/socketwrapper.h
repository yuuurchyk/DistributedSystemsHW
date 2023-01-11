#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "proto2/signal.h"

#include "socketwrapper/buffer.h"

namespace Proto2
{
/**
 * @note writeFrame() is thread safe
 * @note run() does not prolong the lifetime of SocketWrapper
 */
class SocketWrapper : public std::enable_shared_from_this<SocketWrapper>, private logger::StringIdEntity<SocketWrapper>
{
    DISABLE_COPY_MOVE(SocketWrapper)
public:
    /**
     * @param id - SocketWrapper id (for logs)
     * @param executionContext - io context that @p socket runs on
     * @param socket
     * @return std::shared_ptr<SocketWrapper>
     */
    [[nodiscard]] static std::shared_ptr<SocketWrapper>
        create(std::string id, boost::asio::io_context &executionContext, boost::asio::ip::tcp::socket socket);
    ~SocketWrapper();

    using WriteFrameCallback_fn = std::function<void(const boost::system::error_code &, size_t)>;

    /**
     * @brief
     *
     * @note thread safe
     *
     * @param frame - const buffer sequence to be sent over the socket
     * @param callback - callback is expected to capture the data referenced in @p frame for the buffer
     * to remain valid. The callback is called when @p frame was successfully/unsuccessfully written through socket
     * in the thread that runs the execution context
     */
    void writeFrame(std::vector<boost::asio::const_buffer> frame, WriteFrameCallback_fn callback);

    /**
     * @note does not prolong the lifetime of SocketWrapper
     */
    void run();

    void invalidate();

    bool wasInvalidated() const;

    boost::asio::io_context &executionContext();

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

    SocketWrapper(std::string id, boost::asio::io_context &, boost::asio::ip::tcp::socket);

    void readFrameSize();
    void readFrame();

    void checkPendingWrites();

private:
    boost::asio::io_context     &executionContext_;
    boost::asio::ip::tcp::socket socket_;

    bool invalidated_{};

    static constexpr size_t kIncomingBufferMaxReuseSize{ 128 };
    size_t                  incomingFrameSize_{};
    Buffer                  incomingFrameBuffer_;

    bool                     writeInProgress_{};
    size_t                   outcomingFrameSize_;
    std::queue<PendingWrite> pendingWrites_;
};

}    // namespace Proto2
