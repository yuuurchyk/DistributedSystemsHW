#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>

#include "logger/logger.h"

using namespace boost::asio;
using error_code = boost::system::error_code;

class ClientSession : public std::enable_shared_from_this<ClientSession>,
                      private logger::IdEntity<ClientSession>
{
public:
    static std::shared_ptr<ClientSession> create(io_context        &context,
                                                 ip::tcp::endpoint  endpoint,
                                                 const std::string &message)
    {
        return std::shared_ptr<ClientSession>(
            new ClientSession{ context, endpoint, message });
    }

    void start()
    {
        socket_.async_connect(
            endpoint_,
            [this, _thisCtx = shared_from_this()](const error_code &error)
            {
                if (error)
                {
                    ID_LOGI << "Failed to connect to endpoint";
                    return;
                }
                else
                {
                    ID_LOGI << "Successfully connected to endpoint";
                    writeMessage();
                }
            });
    }

private:
    ClientSession(io_context        &context,
                  ip::tcp::endpoint  endpoint,
                  const std::string &message)
        : context_{ context }, endpoint_{ std::move(endpoint) }, socket_{ context_ }
    {
        std::copy(message.begin(),
                  message.begin() + std::min(message.size(), message_.size()),
                  message_.data());
        messageSize_ = std::min(message_.size(), message.size());
    }

    void writeMessage()
    {
        ID_LOGI << "Sending message to server: "
                << std::string_view{ message_.data(), messageSize_ };
        ID_LOGI << "Last charater is newline: " << std::boolalpha
                << (message_[messageSize_ - 1] == '\n');
        async_write(socket_,
                    buffer(message_.data(), messageSize_),
                    [this, _thisCtx = shared_from_this()](const error_code &error, size_t)
                    {
                        if (error)
                            return;
                        else
                            return onWriteComplete();
                    });
    }

    void onWriteComplete()
    {
        ID_LOGI << "waiting for " << messageSize_ << " bytes";
        async_read(
            socket_,
            mutable_buffer{ message_.data(), messageSize_ },
            transfer_exactly(messageSize_),
            [this, _thisCtx = shared_from_this()](const error_code &error, size_t bytes)
            { readHandler(error, bytes); });
    }

    void readHandler(const error_code &error, size_t bytes)
    {
        if (error)
        {
            ID_LOGI << "Failed to retrieve message from server";
            return;
        }
        else
        {
            ID_LOGI << "Server returned " << bytes << " bytes";
            ID_LOGI << "Message returned from server: "
                    << std::string_view{ message_.data(), messageSize_ };
        }
    }

    io_context             &context_;
    const ip::tcp::endpoint endpoint_;
    ip::tcp::socket         socket_;
    size_t                  messageSize_{};
    std::array<char, 1024>  message_;
};

int main()
{
    logger::setup("client");

    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    auto context = io_context{};

    const auto endpoint =
        ip::tcp::endpoint{ ip::address::from_string("127.0.0.1"), 8001 };

    ClientSession::create(context, endpoint, "hello?\n")->start();
    ClientSession::create(context, endpoint, "message2\n")->start();

    context.run();

    return 0;
}
