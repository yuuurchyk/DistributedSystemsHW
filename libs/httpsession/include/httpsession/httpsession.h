#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "constants/constants.h"
#include "logger/logger.h"

class HttpSession : public std::enable_shared_from_this<HttpSession>,
                    protected logger::IdEntity<HttpSession>
{
    DISABLE_COPY_MOVE(HttpSession);

public:
    virtual ~HttpSession() = default;

    void run();

protected:
    HttpSession(boost::asio::ip::tcp::socket socket);

    void setDeadline();
    void readRequest();

    /**
     * @brief don't forget to call writeResponse() at the end
     * of this function
     */
    virtual void processRequest() = 0;

    void writeResponse();

    boost::asio::ip::tcp::socket socket_;

    static constexpr boost::posix_time::milliseconds kSessionDeadline{
        constants::kHttpRequestTimeoutMs
    };
    boost::asio::deadline_timer timer_;

    boost::beast::flat_buffer requestBuffer_{ constants::kHttpRequestBufferSize };
    boost::beast::http::request<boost::beast::http::dynamic_body>  request_;
    boost::beast::http::response<boost::beast::http::dynamic_body> response_;
};
