#pragma once

#include <functional>
#include <memory>

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
class SocketWrapper : public std::enable_shared_from_this<SocketWrapper>,
                      public logger::Entity<SocketWrapper>
{
    DISABLE_COPY_MOVE(SocketWrapper)
public:
    template <typename T>
    using signal = boost::signals2::signal<T>;

    [[nodiscard]] static std::shared_ptr<SocketWrapper>
        create(boost::asio::ip::tcp::socket socket);

    void run();

    void send(std::shared_ptr<Reflection::SerializationContext>);
    template <Reflection::Serializable T>
    void send(std::shared_ptr<const T> ptr);

    signal<void(boost::asio::const_buffer)> incomingBuffer;
    signal<void()>                          invalidated;

private:
    SocketWrapper(boost::asio::ip::tcp::socket socket);

    void readFrameSize();
    void readFrame();

    void invalidate();

    boost::asio::ip::tcp::socket socket_;

    size_t                       frameSizeBuffer_;
    std::unique_ptr<std::byte[]> frameBuffer_;
};

}    // namespace Proto

#include "detail/socketwrapper_impl.h"
