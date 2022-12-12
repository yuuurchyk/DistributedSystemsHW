#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "proto/proto.h"
#include "proto/timestamp.h"
#include "utils/copymove.h"

#include "masternode.h"

/**
 * @brief replicates requests to secondaries. Resulting future returns only when write concern
 * is satisfied. CompositeAddMessageRequest object is destroyed only when recieved confirmation from all
 * connected secondary nodes.
 */
class CompositeAddMessageRequest : public std::enable_shared_from_this<CompositeAddMessageRequest>,
                                   private logger::StringIdEntity<CompositeAddMessageRequest>
{
    DISABLE_COPY_MOVE(CompositeAddMessageRequest)
public:
    /**
     * @brief
     *
     * @param ioContext         - io context to perform CompositeAddMessageRequest operations in
     * @param weakMasterNode    - weak connection to MasterNode
     * @param message           - message request
     * @param writeConcern      - number of confirmations from secondary nodes before setting promise to true
     *
     * @return std::shared_ptr<CompositeAddMessageRequest>
     */
    [[nodiscard]] static std::shared_ptr<CompositeAddMessageRequest> create(
        boost::asio::io_context  &ioContext,
        std::weak_ptr<MasterNode> weakMasterNode,
        Proto::Message            message,
        size_t                    writeConcern);

    ~CompositeAddMessageRequest();

    boost::future<bool> getFuture();

    void run();

private:
    CompositeAddMessageRequest(boost::asio::io_context &, std::weak_ptr<MasterNode>, Proto::Message, size_t);

    void sendRequestToSecondaries();
    void onResponseRecieved(size_t secondaryId, boost::future<Proto::Response::AddMessage>);

    void setPromiseValue();
    std::shared_ptr<MasterNode> lockMaster();

    boost::asio::io_context  &ioContext_;
    std::weak_ptr<MasterNode> weakMasterNode_;

    const Proto::Message message_;

    std::unordered_map<size_t, MasterNode::SecondaryStatus> successes_;
    std::unordered_map<size_t, MasterNode::SecondaryStatus> pending_;
    size_t                                                  cnt_{};

    // timer is activated when write concern is not satisfied, but we still need to
    // wait for confirmations from secondaries
    static constexpr boost::posix_time::milliseconds kDelay{ 3000 };
    boost::asio::deadline_timer                      waitTimer_;

    bool                 invalidated_{};
    bool                 promiseSet_{};
    boost::promise<bool> promise_;

    const size_t writeConcern_;
};
