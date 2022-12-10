#pragma once

#include <functional>
#include <memory>
#include <utility>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "logger/logger.h"
#include "reflection/concepts.h"
#include "reflection/serialization.h"
#include "utils/copymove.h"

namespace Proto
{
/**
 * @brief handles socket reads/writes in terms of frames
 *
 * When send is called, the size of frame is automatically calculated and added
 * to the beginning of the frame (NO need to do this by hand). Same is happening
 * on the side of reading
 */
class SocketWrapper : public std::enable_shared_from_this<SocketWrapper>, public logger::Entity<SocketWrapper>
{
    DISABLE_COPY_MOVE(SocketWrapper)
public:
    template <typename T>
    using signal = boost::signals2::signal<T>;

    /**
     * @param ioContext - io context, associated with @p socket
     * @param socket
     */
    [[nodiscard]] static std::shared_ptr<SocketWrapper>
        create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::socket socket);

    void run();

    // thread safe
    void send(std::shared_ptr<Reflection::SerializationContext>);
    template <Reflection::Serializable T>
    void send(std::shared_ptr<const T> ptr);

    signal<void(boost::asio::const_buffer)> incomingBuffer;
    signal<void()>                          invalidated;

    void invalidate();

    boost::asio::io_context &ioContext();

private:
    SocketWrapper(boost::asio::io_context &, boost::asio::ip::tcp::socket);

    void readFrameSize();
    void readFrame();

    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context     &ioContext_;

    size_t frameSize_;

    std::unique_ptr<std::byte[]> buffer_{};
    size_t                       bufferSize_{};
};

template <Reflection::Serializable T>
void SocketWrapper::send(std::shared_ptr<const T> ptr)
{
    if (ptr == nullptr)
        return;

    auto context = std::make_shared<Reflection::SerializationContext>();
    context->serializeAndHold(std::move(ptr));

    send(std::move(context));
}

}    // namespace Proto
