#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

#include "messages.h"

class HttpSession : public std::enable_shared_from_this<HttpSession>,
                    private logger::IdEntity<HttpSession>
{
    DISABLE_COPY_MOVE(HttpSession);

public:
    static std::shared_ptr<HttpSession> create(boost::asio::ip::tcp::socket socket);

    void run();

private:
    HttpSession(boost::asio::ip::tcp::socket socket);

    void setDeadline();
    void readRequest();
    void processRequest();

    void writeResponse();

    boost::asio::ip::tcp::socket socket_;

    static constexpr boost::posix_time::milliseconds kSessionDeadline{ 5000 };
    boost::asio::deadline_timer                      timer_;

    boost::beast::flat_buffer                                      requestBuffer_{ 1024 };
    boost::beast::http::request<boost::beast::http::dynamic_body>  request_;
    boost::beast::http::response<boost::beast::http::dynamic_body> response_;
};
