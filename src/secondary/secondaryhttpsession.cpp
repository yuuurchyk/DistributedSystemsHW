#include "secondaryhttpsession.h"

#include <boost/json.hpp>

#include "messages.h"

using namespace boost::asio;
using namespace boost::beast;

std::shared_ptr<SecondaryHttpSession> SecondaryHttpSession::create(ip::tcp::socket socket)
{
    return std::shared_ptr<SecondaryHttpSession>(
        new SecondaryHttpSession{ std::move(socket) });
}

SecondaryHttpSession::SecondaryHttpSession(ip::tcp::socket socket)
    : HttpSession{ std::move(socket) }
{
}

void SecondaryHttpSession::processRequest()
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
