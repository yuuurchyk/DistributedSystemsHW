#include "net-utils/httpsession.h"

#include <algorithm>
#include <utility>

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace boost::asio;
using namespace boost::beast;

namespace NetUtils
{
void HttpSession::run()
{
    setDeadline();
    readRequest();
}

HttpSession::HttpSession(ip::tcp::socket socket) : socket_{ std::move(socket) } {}
HttpSession::HttpSession(ip::tcp::socket socket, size_t timeoutMs)
    : socket_{ std::move(socket) },
      timeoutMs_{ std::max<size_t>(timeoutMs, 1) },
      timer_{ deadline_timer{ socket_.get_executor() } }
{
}

void HttpSession::setDeadline()
{
    if (!timer_.has_value())
        return;

    auto &timer = timer_.value();

    EN_LOGI << "setting deadline timer";
    timer.expires_from_now(boost::posix_time::milliseconds{ timeoutMs_.value() });
    timer.async_wait(
        [this, self = shared_from_this()](const error_code &ec)
        {
            if (ec)
                return;

            EN_LOGE << "timeout occured for request";
            socket_.close();
        });
}

void HttpSession::readRequest()
{
    EN_LOGI << "reading request";
    http::async_read(
        socket_,
        requestBuffer_,
        request_,
        [this, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (ec)
            {
                EN_LOGE << "failed to read request";
                if (timer_.has_value())
                    timer_.value().cancel();
            }
            else
            {
                EN_LOGI << "successfully read request";
                processRequest();
            }
        });
}

void HttpSession::writeResponse()
{
    http::async_write(
        socket_,
        response_,
        [this, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (timer_.has_value())
                timer_.value().cancel();
        });
}

}    // namespace NetUtils