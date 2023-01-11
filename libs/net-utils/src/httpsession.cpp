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

HttpSession::HttpSession(boost::asio::io_context &ioContext, ip::tcp::socket socket)
    : ioContext_{ ioContext }, socket_{ std::move(socket) }
{
}
HttpSession::HttpSession(boost::asio::io_context &ioContext, ip::tcp::socket socket, size_t timeoutMs)
    : ioContext_{ ioContext },
      socket_{ std::move(socket) },
      timeoutMs_{ std::max<size_t>(timeoutMs, 1) },
      timer_{ deadline_timer{ socket_.get_executor() } }
{
}

void HttpSession::setDeadline()
{
    if (!timer_.has_value())
        return;

    auto &timer = timer_.value();

    EN_LOGD << "setting deadline timer";
    timer.expires_from_now(boost::posix_time::milliseconds{ timeoutMs_.value() });
    timer.async_wait(
        [this, self = shared_from_this()](const error_code &ec)
        {
            if (ec)
                return;

            EN_LOGW << "timeout occured for request";
            socket_.close();
        });
}

void HttpSession::readRequest()
{
    EN_LOGD << "reading request";
    http::async_read(
        socket_,
        requestBuffer_,
        request_,
        [this, self = shared_from_this()](const error_code &ec, size_t)
        {
            if (ec)
            {
                EN_LOGW << "failed to read request";
                if (timer_.has_value())
                    timer_.value().cancel();
            }
            else
            {
                EN_LOGD << "successfully read request";
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

void HttpSession::fallback(std::string reason)
{
    EN_LOGW << "invalid request, falling back";
    response_.result(http::status::bad_request);
    response_.set(http::field::content_type, "text/plain");
    response_.body() = std::move(reason);
    response_.content_length(response_.body().size());
    writeResponse();
}

}    // namespace NetUtils
