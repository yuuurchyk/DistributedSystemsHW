#include "secondaryhttpsession.h"

#include <utility>

#include <boost/beast.hpp>
#include <boost/json.hpp>

using namespace boost::beast;

std::shared_ptr<SecondaryHttpSession> SecondaryHttpSession::create(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    std::weak_ptr<SecondaryNode> secondaryNode)
{
    return std::shared_ptr<SecondaryHttpSession>{ new SecondaryHttpSession{
        ioContext, std::move(socket), std::move(secondaryNode) } };
}

SecondaryHttpSession::SecondaryHttpSession(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    std::weak_ptr<SecondaryNode> weakNode)
    : NetUtils::HttpSession{ ioContext, std::move(socket) }, weakNode_{ std::move(weakNode) }
{
}

void SecondaryHttpSession::processRequest()
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
            auto messages = node->getMessages();

            auto array = boost::json::array{};
            array.reserve(messages.size());

            for (const auto &message : messages)
            {
                auto obj         = boost::json::object{};
                obj["timestamp"] = message.timestamp;
                obj["message"]   = boost::json::string_view{ message.message };
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
