#include <functional>
#include <memory>
#include <shared_mutex>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/scope_exit.hpp>

#include "communicationendpoint/communicationendpoint.h"
#include "iocontextpool/iocontextpool.h"
#include "logger/logger.h"
#include "messagesstorage/messagesstorage.h"
#include "utils/copymove.h"

using error_code = boost::system::error_code;
using namespace boost::asio;
using namespace boost::beast;

MessagesStorage messagesStorage;

class HttpSession : public std::enable_shared_from_this<HttpSession>,
                    private logger::IdEntity<HttpSession>
{
    DISABLE_COPY_MOVE(HttpSession);

public:
    static std::shared_ptr<HttpSession> create(ip::tcp::socket socket)
    {
        return std::shared_ptr<HttpSession>(new HttpSession{ std::move(socket) });
    }

    void run()
    {
        setDeadline();
        readRequest();
    }

private:
    HttpSession(ip::tcp::socket socket)
        : socket_{ std::move(socket) }, timer_{ socket_.get_executor() }
    {
    }

    void setDeadline()
    {
        ID_LOGI << "setting deadline timer";
        timer_.expires_from_now(boost::posix_time::milliseconds{ 5000 });
        timer_.async_wait(
            [this, self = shared_from_this()](const error_code &error)
            {
                if (error)
                    return;

                ID_LOGE << "timeout occured for request";
                socket_.close();
            });
    }

    void readRequest()
    {
        ID_LOGI << "reading request";
        http::async_read(
            socket_,
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

    void processRequest()
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

            auto messages = messagesStorage.getMessages();

            auto array = boost::json::array{};
            array.reserve(messages.size());

            for (auto &message : messages)
                array.push_back(boost::json::string_view{ message });

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

    void writeResponse()
    {
        http::async_write(socket_,
                          response_,
                          [this, self = shared_from_this()](const error_code &error,
                                                            size_t) { timer_.cancel(); });
    }

    ip::tcp::socket socket_;
    deadline_timer  timer_;

    flat_buffer                        requestBuffer_{ 8192 };
    http::request<http::dynamic_body>  request_;
    http::response<http::dynamic_body> response_;
};

class HttpServer : public std::enable_shared_from_this<HttpServer>
{
    DISABLE_COPY_MOVE(HttpServer)
public:
    static std::shared_ptr<HttpServer> create(io_context              &context,
                                              const ip::tcp::endpoint &endpoint,
                                              IOContextPool           &httpWorkersPool)
    {
        return std::shared_ptr<HttpServer>(
            new HttpServer{ context, endpoint, httpWorkersPool });
    }

    void run() { acceptIncomingConnection(); }

private:
    HttpServer(io_context              &context,
               const ip::tcp::endpoint &endpoint,
               IOContextPool           &httpWorkersPool)
        : acceptor_{ context, endpoint }, pool_{ httpWorkersPool }
    {
    }

    void acceptIncomingConnection()
    {
        auto  socketOwner = std::make_unique<ip::tcp::socket>(pool_.getNext());
        auto &socket      = *socketOwner;

        acceptor_.async_accept(
            socket,
            [self        = shared_from_this(),
             socketOwner = std::move(socketOwner)](const error_code &error)
            {
                if (error)
                {
                    LOGE << "Could not accept incoming connection";
                    return;
                }

                LOGI << "Accepting incoming connection";

                HttpSession::create(std::move(*socketOwner))->run();

                self->acceptIncomingConnection();
            });
    }

    ip::tcp::acceptor acceptor_;
    IOContextPool    &pool_;
};

int main()
{
    logger::setup("secondary");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    messagesStorage.addMessage(std::string{ "jehh" });
    messagesStorage.addMessage(std::string{ "jehhd" });
    messagesStorage.addMessage(std::string{ "jehhffds" });
    messagesStorage.addMessage(std::string{ "jehhddddddd" });
    const auto port           = 8000;
    const auto httpWorkersNum = 3;

    auto httpPool = IOContextPool{ httpWorkersNum };
    httpPool.runInSeparateThreads();

    auto httpServerContext = io_context{};
    HttpServer::create(
        httpServerContext, ip::tcp::endpoint{ ip::tcp::v4(), port }, httpPool)
        ->run();
    httpServerContext.run();

    httpPool.stop();

    return 0;
}
