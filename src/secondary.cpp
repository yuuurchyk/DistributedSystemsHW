#include <functional>
#include <memory>
#include <shared_mutex>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/scope_exit.hpp>

#include <tbb/concurrent_vector.h>

#include "iocontextpool/iocontextpool.h"
#include "logger/logger.h"
#include "socketacceptor/socketacceptor.h"
#include "utils/copymove.h"

using error_code = boost::system::error_code;
using namespace boost::asio;
using namespace boost::beast;

tbb::concurrent_vector<std::string> messages;

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

int main()
{
    logger::setup("secondary");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    messages.push_back("a");
    messages.push_back("b");

    const auto port           = 8000;
    const auto httpWorkersNum = 3;

    auto httpPool = IOContextPool{ httpWorkersNum };
    httpPool.runInSeparateThreads();

    auto acceptorContext = io_context{};
    SocketAcceptor::create(
        acceptorContext,
        port,
        [](ip::tcp::socket socket) { HttpSession::create(std::move(socket))->run(); },
        httpPool,
        SocketAcceptor::IOContextSelectionPolicy::Random)
        ->run();

    acceptorContext.run();
    httpPool.stop();

    return 0;
}
