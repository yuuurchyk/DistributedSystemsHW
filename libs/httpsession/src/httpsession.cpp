#include "httpsession/httpsession.h"

using namespace boost::asio;
using namespace boost::beast;

void HttpSession::run()
{
    setDeadline();
    readRequest();
}

HttpSession::HttpSession(ip::tcp::socket socket)
    : socket_{ std::move(socket) }, timer_{ socket_.get_executor() }
{
}

void HttpSession::setDeadline()
{
    ID_LOGI << "setting deadline timer";
    timer_.expires_from_now(kSessionDeadline);
    timer_.async_wait(
        [this, self = shared_from_this()](const error_code &error)
        {
            if (error)
                return;

            ID_LOGE << "timeout occured for request";
            socket_.close();
        });
}

void HttpSession::readRequest()
{
    ID_LOGI << "reading request";
    http::async_read(socket_,
                     requestBuffer_,
                     request_,
                     [this, self = shared_from_this()](const error_code &error, size_t)
                     {
                         if (error)
                         {
                             ID_LOGE << "failed to read request";
                             timer_.cancel();
                         }
                         else
                         {
                             ID_LOGI << "successfully read request";
                             processRequest();
                         }
                     });
}

void HttpSession::writeResponse()
{
    http::async_write(socket_,
                      response_,
                      [this, self = shared_from_this()](const error_code &error, size_t)
                      { timer_.cancel(); });
}
