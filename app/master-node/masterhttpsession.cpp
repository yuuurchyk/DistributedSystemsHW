#include "masterhttpsession.h"

#include <boost/beast.hpp>
#include <boost/json.hpp>

#include <exception>
#include <utility>

using namespace boost::beast;

std::shared_ptr<MasterHttpSession> MasterHttpSession::create(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    std::weak_ptr<MasterNode>    weakNode)
{
    return std::shared_ptr<MasterHttpSession>{ new MasterHttpSession{
        ioContext, std::move(socket), std::move(weakNode) } };
}

void MasterHttpSession::processRequest()
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

        response_.content_length(response_.body().size());
        return writeResponse();
    }
    else if (request_.method() == http::verb::post && request_.target() == "/newMessage")
    {
        auto message      = std::string{};
        auto writeConcern = size_t{};

        try
        {
            auto objBody          = boost::json::parse(request_.body()).as_object();
            auto jsonWriteConcern = objBody.at("writeConcern").as_int64();
            auto jsonMessage      = objBody.at("message").as_string();

            if (jsonWriteConcern < 0)
                throw std::runtime_error{ "Write concern cannot be negative" };

            message = std::string{ jsonMessage.data(), jsonMessage.size() };
            static_assert(sizeof(size_t) == sizeof(uint64_t));
            writeConcern = static_cast<size_t>(jsonWriteConcern);
        }
        catch (const std::exception &e)
        {
            return fallback(std::string{ "Internal error occured: " } + e.what());
        }

        auto node = weakNode_.lock();
        if (node == nullptr)
            return fallback("node is dead (should not happen)");

        node->addMessage(ioContext_, std::move(message), writeConcern)
            .then(
                [this, self = shared_from_this()](boost::future<bool> responseFuture)
                {
                    boost::asio::post(
                        ioContext_,
                        [this, responseFuture = std::move(responseFuture), self]() mutable
                        {
                            if (!responseFuture.has_value())
                                return fallback("error occured for request");
                            if (!responseFuture.get())
                                return fallback("error occured for request");
                            response_.body() = "ok";
                            response_.content_length(response_.body().size());
                            writeResponse();
                        });
                });

        EN_LOGI << "valid request, adding new message: " << message << ", writeConcern=" << writeConcern;
    }
    else
    {
        return fallback("invalid request");
    }
}

MasterHttpSession::MasterHttpSession(
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    std::weak_ptr<MasterNode>    weakNode)
    : NetUtils::HttpSession{ ioContext, std::move(socket) }, weakNode_{ std::move(weakNode) }
{
}
