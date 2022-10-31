#pragma once

#include <string>

#include <boost/asio.hpp>

#include "httpsession/httpsession.h"
#include "secondariespool.h"

class MasterHttpSession final : public HttpSession
{
public:
    static std::shared_ptr<MasterHttpSession> create(std::shared_ptr<SecondariesPool>,
                                                     boost::asio::ip::tcp::socket socket);

private:
    MasterHttpSession(std::shared_ptr<SecondariesPool>,
                      boost::asio::ip::tcp::socket socket);

    void processRequest() override;
    void handleAddMessage(std::string message);

    std::shared_ptr<SecondariesPool> secondariesPool_;
};
