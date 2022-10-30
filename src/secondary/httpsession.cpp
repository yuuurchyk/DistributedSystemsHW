#include "httpsession.h"

#include <boost/json.hpp>

using namespace boost::asio;
using namespace boost::beast;

std::shared_ptr<HttpSession> HttpSession::create(ip::tcp::socket socket)
{
    return std::shared_ptr<HttpSession>(new HttpSession{ std::move(socket) });
}

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

void HttpSession::processRequest()
{
    ID_LOGI << "processing request";

    response_.version(request_.version());
    response_.keep_alive(false);

    if (request_.method() == http::verb::get && request_.target() == "/messages")
    {
        ID_LOGI << "valid request, retrieving list of messages";
        response_.result(http::status::ok);
        response_.set(http::field::server, "Beast");
        response_.set(http::field::content_type, "application/json");

        auto array = boost::json::array{};

        const auto size = messages.size();
        array.reserve(messages.size());
        for (auto i = size_t{}; i < size; ++i)
            array.push_back(boost::json::string_view{ messages[i] });

        auto strm = boost::beast::ostream(response_.body());
        strm << array;
    }
    else
    {
        ID_LOGI << "invalid request, falling back";
        response_.result(http::status::bad_request);
        response_.set(http::field::content_type, "text/plain");
        boost::beast::ostream(response_.body()) << "Invalid request ";
    }
    response_.content_length(response_.body().size());

    writeResponse();
}

void HttpSession::writeResponse()
{
    http::async_write(socket_,
                      response_,
                      [this, self = shared_from_this()](const error_code &error, size_t)
                      { timer_.cancel(); });
}
