#pragma once

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "net-utils/httpsession.h"

#include "secondarynode.h"

class SecondaryHttpSession final : public NetUtils::HttpSession
{
public:
    [[nodiscard]] static std::shared_ptr<SecondaryHttpSession> create(
        boost::asio::io_context     &ioContext,
        boost::asio::ip::tcp::socket socket,
        std::weak_ptr<SecondaryNode> secondaryNode);

protected:
    void processRequest() override;

private:
    SecondaryHttpSession(boost::asio::io_context &, boost::asio::ip::tcp::socket, std::weak_ptr<SecondaryNode>);

    std::weak_ptr<SecondaryNode> weakNode_;
};
