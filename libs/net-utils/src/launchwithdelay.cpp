#include "net-utils/launchwithdelay.h"

#include <memory>
#include <utility>

namespace NetUtils
{
void launchWithDelay(
    boost::asio::io_context        &ioContext,
    boost::posix_time::milliseconds delay,
    std::function<void()>           callback)
{
    auto timer = std::make_shared<boost::asio::deadline_timer>(ioContext);
    timer->expires_from_now(delay);
    timer->async_wait(
        [callback = std::move(callback), timer](const boost::system::error_code &ec)
        {
            if (ec)
                return;
            if (callback != nullptr)
                callback();
        });
}

}    // namespace NetUtils
