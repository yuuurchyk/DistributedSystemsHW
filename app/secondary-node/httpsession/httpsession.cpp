#include "httpsession/httpsession.h"

#include <utility>

#include <boost/beast.hpp>
#include <boost/json.hpp>

using namespace boost::beast;

std::shared_ptr<HttpSession> HttpSession::create(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    std::weak_ptr<SecondaryNode> secondaryNode)
{
    return std::shared_ptr<HttpSession>{ new HttpSession{ ioContext, std::move(socket), std::move(secondaryNode) } };
}

HttpSession::HttpSession(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    std::weak_ptr<SecondaryNode> weakNode)
    : NetUtils::HttpSession{ ioContext, std::move(socket) }, weakNode_{ std::move(weakNode) }
{
}

void HttpSession::processRequest()
{
    EN_LOGI << "Processing request";

    response_.version(request_.version());
    response_.keep_alive(false);

    if (request_.method() == http::verb::get && request_.target() == "/messages")
    {
        EN_LOGI << "valid request, retrieving list of messages";
        response_.result(http::status::ok);
        response_.set(http::field::server, "Beast");
        response_.set(http::field::content_type, "application/json");

        if (auto node = weakNode_.lock(); node == nullptr)
        {
            return fallback("node is dead (should not happen)");
        }
        else
        {
            if (!node->operational())
                return fallback("node is not operational");

            auto messages = node->storage().getContiguousChunk();

            auto array = boost::json::array{};
            array.reserve(messages.size());

            for (auto i = size_t{}; i < messages.size(); ++i)
            {
                auto obj       = boost::json::object{};
                obj["id"]      = i;
                obj["message"] = boost::json::string_view{ messages[i] };
                array.push_back(std::move(obj));
            }

            response_.body() = boost::json::serialize(array);
        }
    }
    else
    {
        return fallback("invalid request");
    }
    response_.content_length(response_.body().size());

    writeResponse();
}
