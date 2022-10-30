#pragma once

#include <boost/asio.hpp>

#include "httpsession/httpsession.h"

class MasterHttpSession final : public HttpSession
{
public:
    static std::shared_ptr<MasterHttpSession> create(boost::asio::ip::tcp::socket socket);

private:
    MasterHttpSession(boost::asio::ip::tcp::socket socket);

    void processRequest() override;
};
