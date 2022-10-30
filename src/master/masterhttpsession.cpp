#include "masterhttpsession.h"

#include <utility>

#include <boost/json.hpp>

#include "logger/logger.h"

#include "messages.h"

using namespace boost::asio;
using namespace boost::beast;

std::shared_ptr<MasterHttpSession>
    MasterHttpSession::create(boost::asio::ip::tcp::socket socket)
{
    return std::shared_ptr<MasterHttpSession>(new MasterHttpSession{ std::move(socket) });
}

MasterHttpSession::MasterHttpSession(ip::tcp::socket socket)
    : HttpSession{ std::move(socket) }
{
}

void MasterHttpSession::processRequest()
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

        response_.body() = boost::json::serialize(array);
    }
    else if (request_.method() == http::verb::post && request_.target() == "/addmessage")
    {
        auto message = std::string{ request_.body() };

        ID_LOGI << "valid request, adding the following string: '" << message << "'";

        ::messages.push_back(std::move(message));
    }
    else
    {
        ID_LOGI << "invalid request, falling back";
        response_.result(http::status::bad_request);
        response_.set(http::field::content_type, "text/plain");
        response_.body() = "invalid request";
    }
    response_.content_length(response_.body().size());

    writeResponse();
}
