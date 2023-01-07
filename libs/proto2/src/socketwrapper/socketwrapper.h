#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

namespace Proto2
{
class SocketWrapper : public std::enable_shared_from_this<SocketWrapper>, private logger::Entity<SocketWrapper>
{
public:
    /**
     * @brief provides functionality of sending and recieving frames through
     * a socket
     *
     * @param ioContext - io context object associated with @p socket
     * @param socket
     * @return std::shared_ptr<SocketWrapper>
     */
    [[nodiscard]] static std::shared_ptr<SocketWrapper>
        create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket);

    ~SocketWrapper();

    boost::signals2::signal<void(boost::asio::const_buffer)> incomingFrame;
    boost::signals2::signal<void()>                          invalidated;

    void run();

    void invalidate();

    boost::asio::io_context &ioContext();

    /**
     * @brief
     *
     * @note this method is thread safe
     *
     * @param frame - buffer sequence to send as a frame
     * @param callback - should hold the data referenced in @p frame
     */
    void writeFrame(
        std::vector<boost::asio::const_buffer>                         frame,
        std::function<void(const boost::system::error_code &, size_t)> callback);

private:
    SocketWrapper(boost::asio::io_context &, boost::asio::ip::tcp::socket);

    void readFrameSize();
    void readFrame();

    void writeFrameImpl();

    void allocateBuffer(size_t bytes);

private:
    boost::asio::io_context     &ioContext_;
    boost::asio::ip::tcp::socket socket_;

    bool invalidated_{};

    size_t outcomingFrameSizeBuffer_;

    // if buffer gets bigger than kMaxReusedBufferSize, it
    // shrinks on the next iteration
    static constexpr size_t kMaxReusedBufferSize{ 128 };
    size_t                  incomingFrameSize_{};

    size_t                       bufferSize_{};
    std::unique_ptr<std::byte[]> buffer_{};

    struct FrameContext
    {
        DISABLE_COPY_DEFAULT_MOVE(FrameContext)

        FrameContext(
            std::vector<boost::asio::const_buffer>,
            std::function<void(const boost::system::error_code &, size_t)>);

        std::vector<boost::asio::const_buffer>                         seq;
        std::function<void(const boost::system::error_code &, size_t)> callback;
    };

    std::queue<FrameContext> pendingFrames_;
    bool                     writeInProgress_{};
};

}    // namespace Proto2
