#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "logger/logger.h"
#include "proto/proto.h"
#include "proto/timestamp.h"
#include "utils/copymove.h"

#include "masternode.h"

class CompositePingRequest : public std::enable_shared_from_this<CompositePingRequest>,
                             private logger::NumIdEntity<CompositePingRequest>
{
    DISABLE_COPY_MOVE(CompositePingRequest)
public:
    /**
     * @brief                   - query secondaries only one time, don't account for newly connected nodes
     *                             while the request was pending
     * @param ioContext         - io context to perform CompositePingRequest operations in
     * @param weakMasterNode    - weak connection to MasterNode
     */
    [[nodiscard]] static std::shared_ptr<CompositePingRequest>
        create(boost::asio::io_context &ioContext, std::weak_ptr<MasterNode> weakMasterNode);

    /**
     * @return boost::future<std::optional<std::string>> - already processed json response as string
     */
    boost::future<std::optional<std::string>> getFuture();

    void run();

private:
    CompositePingRequest(boost::asio::io_context &, std::weak_ptr<MasterNode>);

    void sendRequestToSecondaries();
    void onAllResponsesRecieved(boost::future<std::vector<boost::future<Proto::Response::Pong>>>);

    boost::asio::io_context  &ioContext_;
    std::weak_ptr<MasterNode> weakMasterNode_;

    Proto::Timestamp_t sendTimestamp_;

    std::vector<size_t>                               ids_;
    std::vector<MasterNode::SecondaryStatus>          statuses_;
    std::vector<std::string>                          friendlyNames_;
    std::vector<boost::future<Proto::Response::Pong>> futures_;

    boost::promise<std::optional<std::string>> promise_;
};
