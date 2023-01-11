#pragma once

#include <boost/asio.hpp>

#include "net-utils/httpsession.h"

#include "secondarynode/secondarynode.h"

class HttpSession final : public NetUtils::HttpSession
{
public:
    [[nodiscard]] static std::shared_ptr<HttpSession> create(
        boost::asio::io_context     &ioContext,
        boost::asio::ip::tcp::socket socket,
        std::weak_ptr<SecondaryNode> secondaryNode);

protected:
    void processRequest() override;

private:
    HttpSession(boost::asio::io_context &, boost::asio::ip::tcp::socket, std::weak_ptr<SecondaryNode>);

    std::weak_ptr<SecondaryNode> weakNode_;
};
