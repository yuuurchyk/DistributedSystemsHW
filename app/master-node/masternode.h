#pragma once

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "proto/communicationendpoint.h"
#include "proto/proto.h"
#include "proto/timestamp.h"
#include "utils/copymove.h"

class CompositeAddMessageRequest;

class MasterNode : public std::enable_shared_from_this<MasterNode>, private logger::Entity<MasterNode>
{
    friend class CompositeAddMessageRequest;
    DISABLE_COPY_MOVE(MasterNode)
public:
    /**
     * @param ioContext         - io context for MasterNode (i.e. accepting and watching over secondaries)
     * @param port              - port where to accept incoming secondary nodes connections
     * @param secondariesPool   - pool of io contexts to use when creating secondary nodes
     * @return std::shared_ptr<MasterNode>
     */
    [[nodiscard]] static std::shared_ptr<MasterNode> create(
        boost::asio::io_context                 &ioContext,
        unsigned short                           port,
        std::shared_ptr<NetUtils::IOContextPool> secondariesPool);

    ~MasterNode();

    void run();

    /**
     * @param writeConcern - number of confirmation from secondaries before considering request a success
     * @note resulting future will never be populated with result if @p writeConcern is bigger than the number
     *      of connected secondary nodes
     */
    boost::future<bool> addMessage(Proto::Timestamp_t, std::string, size_t writeConcern);

private:
    enum class SecondaryStatus
    {
        Registering,
        Active
    };

    struct Secondary
    {
        DISABLE_COPY_MOVE(Secondary)

        size_t                                        id{};
        SecondaryStatus                               status{ SecondaryStatus::Registering };
        std::shared_ptr<Proto::CommunicationEndpoint> endpoint{};
    };

    size_t getNextSecondaryId();

    std::atomic<size_t>                                    secondaryCounter_;
    std::shared_mutex                                      secondariesMutex_;
    std::unordered_map<size_t, std::unique_ptr<Secondary>> secondaries_;
};
