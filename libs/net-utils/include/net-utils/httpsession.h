#pragma once

#include <memory>
#include <optional>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "logger/logger.h"
#include "utils/copymove.h"

namespace NetUtils
{
class HttpSession : public std::enable_shared_from_this<HttpSession>, protected logger::NumIdEntity<HttpSession>
{
    DISABLE_COPY_MOVE(HttpSession)

public:
    virtual ~HttpSession() = default;

    void run();

protected:
    HttpSession(boost::asio::io_context &, boost::asio::ip::tcp::socket socket);
    HttpSession(boost::asio::io_context &, boost::asio::ip::tcp::socket socket, size_t timeoutMs);

    /**
     * @note don't forget to call writeResponse() at the end
     * of this function
     */
    virtual void processRequest() = 0;

    void writeResponse();

    boost::asio::io_context &ioContext_;

private:
    void setDeadline();
    void readRequest();

    boost::asio::ip::tcp::socket socket_;

    std::optional<size_t>                      timeoutMs_;
    std::optional<boost::asio::deadline_timer> timer_;

    boost::beast::flat_buffer                                     requestBuffer_{ 1024 };
    boost::beast::http::request<boost::beast::http::string_body>  request_;
    boost::beast::http::response<boost::beast::http::string_body> response_;
};

}    // namespace NetUtils
