#pragma once

#include <boost/asio.hpp>

#include "httpsession/httpsession.h"

class SecondaryHttpSession final : public HttpSession
{
public:
    static std::shared_ptr<SecondaryHttpSession>
        create(boost::asio::ip::tcp::socket socket);

private:
    SecondaryHttpSession(boost::asio::ip::tcp::socket socket);

    void processRequest() override;
};
