#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"

using namespace boost::asio;
using error_code = boost::system::error_code;

class ServerSession : public std::enable_shared_from_this<ServerSession>,
                      private logger::IdEntity<ServerSession>
{
public:
    static std::shared_ptr<ServerSession> create(ip::tcp::socket socket)
    {
        return std::shared_ptr<ServerSession>{ new ServerSession{ std::move(socket) } };
    }

    void start()
    {
        ID_LOGI << "ServerSession::start";
        async_read(
            socket_,
            mutable_buffer{ message_.data(), message_.size() },
            [this, _thisCtx = shared_from_this()](const error_code &error,
                                                  size_t            bytesTransferred)
            { return readCompletionHandler(error, bytesTransferred); },
            [this, _thisCtx = shared_from_this()](const error_code &error, size_t bytes)
            { return readHandler(error, bytes); });
    }

private:
    ServerSession(ip::tcp::socket socket) : socket_{ std::move(socket) } {}

    size_t readCompletionHandler(const error_code &error, size_t bytesTransferred)
    {
        ID_LOGI << "readCompletionHandler " << bytesTransferred;
        if (error)
        {
            ID_LOGI << "error, return 0";
            return 0;
        }

        if (bytesTransferred == message_.size())
        {
            ID_LOGI << "message too big, return 0";
            return 0;
        }

        for (; checkedOffset_ < message_.size() && checkedOffset_ < bytesTransferred;
             ++checkedOffset_)
        {
            ID_LOGI << "checking offset " << checkedOffset_
                    << ", character=" << message_[checkedOffset_];
            if (message_[checkedOffset_] == '\n')
            {
                ++checkedOffset_;
                ID_LOGI << "found newline, return 0";
                return 0;
            }
        }

        ID_LOGI << "need to reed one more character, return 1";
        return 1;
    }

    void readHandler(const error_code &error, size_t bytes)
    {
        ID_LOGI << "inside read handler, text="
                << std::string_view{ message_.data(), bytes };
        if (error)
        {
            ID_LOGI << "error, returning";
            return;
        }

        std::transform(message_.data(),
                       message_.data() + checkedOffset_,
                       message_.data(),
                       [](unsigned char c) { return std::toupper(c); });

        async_write(
            socket_,
            buffer(message_.data(), checkedOffset_),
            [this, _thisCtx = shared_from_this()](const error_code &error, size_t bytes)
            {
                if (error)
                {
                    ID_LOGI << "write failed";
                }
                else
                {
                    ID_LOGI << "written " << bytes << " bytes back to client";
                }
            });
    }

    ip::tcp::socket socket_;

    std::array<char, 1024> message_;
    size_t                 checkedOffset_{};
};

class Server : public std::enable_shared_from_this<Server>
{
public:
    static std::shared_ptr<Server> create(io_context &context, unsigned short port)
    {
        return std::shared_ptr<Server>(new Server{ context, port });
    }

    void start() { acceptOne(); }

private:
    Server(io_context &context, unsigned short port)
        : context_{ context },
          acceptor_{ context, ip::tcp::endpoint{ ip::tcp::v4(), port } }
    {
        LOGI << "Serving on port " << port;
    }

    void acceptOne()
    {
        LOGI << "start: creating socket";

        auto  socket    = std::make_unique<ip::tcp::socket>(context_);
        auto &socketRef = *socket;

        acceptor_.async_accept(
            socketRef,
            [this, socket = std::move(socket), _thisCtx = shared_from_this()](
                const error_code &error) mutable
            {
                LOGI << "start: async_accept handler";

                if (error)
                {
                    LOGI << "error occured, don't create session";
                }
                else
                {
                    LOGI << "all good, creating session";
                    auto s = ip::tcp::socket{ std::move(*socket.release()) };
                    ServerSession::create(std::move(s))->start();
                }

                acceptOne();
            });
    }

    io_context       &context_;
    ip::tcp::acceptor acceptor_;
};

int main()
{
    logger::setup("server");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "Setting up io context";
    auto context = io_context{};

    Server::create(context, 8001)->start();

    auto threadWorker = [&context]() { return context.run(); };
    auto threads      = std::vector<std::thread>{};
    for (auto i = size_t{}; i < 2; ++i)
        threads.emplace_back(threadWorker);
    threadWorker();
    for (auto &thread : threads)
        thread.join();

    return 0;
}
