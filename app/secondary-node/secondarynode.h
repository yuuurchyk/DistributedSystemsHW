#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <tbb/concurrent_unordered_map.h>

#include "logger/logger.h"
#include "proto/communicationendpoint.h"
#include "proto/proto.h"
#include "proto/timestamp.h"
#include "utils/copymove.h"

class SecondaryNode : public std::enable_shared_from_this<SecondaryNode>, private logger::Entity<SecondaryNode>
{
    DISABLE_COPY_MOVE(SecondaryNode)
public:
    /**
     * @param ioContext             - context for communication with master node
     * @param masterSocketEndpoint  - master endpoint to connect to
     * @return std::shared_ptr<SecondaryNode>
     */
    [[nodiscard]] static std::shared_ptr<SecondaryNode>
        create(boost::asio::io_context &ioContext, boost::asio::ip::tcp::endpoint masterSocketEndpoint);

    void run();

    /**
     * @note thread safe
     */
    std::vector<Proto::Message> getMessages();

private:
    SecondaryNode(boost::asio::io_context &, boost::asio::ip::tcp::endpoint);

    void reconnect();
    void establishMasterEndpoint(boost::asio::ip::tcp::socket);

    void sendGetMessagesRequest(std::weak_ptr<Proto::CommunicationEndpoint>);
    void sendSecondaryNodeReadyRequest(std::weak_ptr<Proto::CommunicationEndpoint>);

    tbb::concurrent_unordered_map<Proto::Timestamp_t, std::string> messages_;

    std::shared_ptr<Proto::CommunicationEndpoint> communicationEndpoint_;

    boost::asio::io_context             &ioContext_;
    const boost::asio::ip::tcp::endpoint masterSocketEndpoint_;

    boost::asio::deadline_timer reconnectTimer_;
};
