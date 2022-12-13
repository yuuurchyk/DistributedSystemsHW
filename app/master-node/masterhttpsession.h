#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "net-utils/httpsession.h"

#include "masternode.h"

class MasterHttpSession final : public NetUtils::HttpSession
{
public:
    [[nodiscard]] static std::shared_ptr<MasterHttpSession> create(
        boost::asio::io_context     &ioContext,
        boost::asio::ip::tcp::socket socket,
        std::weak_ptr<MasterNode>    weakNode);

protected:
    void processRequest() override;

private:
    MasterHttpSession(boost::asio::io_context &, boost::asio::ip::tcp::socket, std::weak_ptr<MasterNode>);

    std::weak_ptr<MasterNode> weakNode_;
};
