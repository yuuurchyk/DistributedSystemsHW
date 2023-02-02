#include "httpsession/httpsession.h"

#include <exception>
#include <sstream>
#include <utility>

#include <boost/beast.hpp>
#include <boost/json.hpp>

#include "net-utils/thenpost.h"

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
    EN_LOGD << "Processing request";

    response_.version(request_.version());
    response_.keep_alive(false);

    if (request_.method() == http::verb::get && request_.target() == "/messages")
    {
        EN_LOGD << "valid request, retrieving list of messages";

        response_.result(http::status::ok);
        response_.set(http::field::server, "Beast");
        response_.set(http::field::content_type, "application/json");

        if (auto node = weakNode_.lock(); node == nullptr)
        {
            return fallback("node is dead (should not happen)");
        }
        else
        {
            const auto messages = node->storage().getContiguousChunk();

            auto array = boost::json::array{};
            array.reserve(messages.size());

            for (auto i = size_t{}; i < messages.size(); ++i)
            {
                auto obj = boost::json::object{};

                obj["id"]      = i;
                obj["message"] = boost::json::string_view{ messages[i] };

                array.push_back(std::move(obj));
            }

            response_.body() = boost::json::serialize(array);
        }

        response_.content_length(response_.body().size());
        return writeResponse();
    }
    else if (request_.method() == http::verb::get && request_.target() == "/pingSecondaries")
    {
        EN_LOGD << "valid request, ping secondaries";

        response_.result(http::status::ok);
        response_.set(http::field::server, "Beast");
        response_.set(http::field::content_type, "application/json");

        auto node = weakNode_.lock();
        if (node == nullptr)
            return fallback("node is dead (should not happen)");

        NetUtils::thenPost(
            node->pingSecondaries(ioContext_),
            ioContext_,
            [this, self = shared_from_this()](boost::future<std::vector<MasterNode::PingResult>> resultFuture)
            {
                if (!resultFuture.has_value())
                    return fallback("error occured for request");

                auto result = resultFuture.get();

                auto resultJson = boost::json::array{};

                for (auto &entry : result)
                {
                    auto obj = boost::json::object{};

                    obj["secondaryId"] = entry.secondaryId;
                    if (entry.secondaryFriendlyName.has_value())
                        obj["friendlyName"] = std::move(entry.secondaryFriendlyName.value());

                    {
                        auto strm = std::stringstream{};
                        strm << entry.secondaryState;
                        obj["state"] = std::move(strm).str();
                    }

                    obj["pingTimestamp"] = entry.pingTimestamp;

                    if (entry.pongTimestamp.has_value())
                        obj["pongTimestamp"] = entry.pongTimestamp.value();
                    if (entry.exceptionString.has_value())
                        obj["exceptionString"] = std::move(entry.exceptionString.value());

                    resultJson.push_back(std::move(obj));
                }

                response_.body() = boost::json::serialize(resultJson);

                response_.content_length(response_.body().size());
                return writeResponse();
            });
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

        EN_LOGD << "valid request, adding new message: " << message << ", writeConcern=" << writeConcern;

        NetUtils::thenPost(
            node->addMessage(ioContext_, std::move(message), writeConcern),
            ioContext_,
            [this, self = shared_from_this()](boost::future<void> response)
            {
                if (!response.has_value())
                    return fallback("error occured for request");

                response_.body() = "ok";
                response_.content_length(response_.body().size());
                writeResponse();
            });
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
