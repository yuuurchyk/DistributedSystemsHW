#pragma once

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <tbb/concurrent_unordered_map.h>

#include "logger/logger.h"
#include "net-utils/iocontextpool.h"
#include "net-utils/socketacceptor.h"
#include "proto/communicationendpoint.h"
#include "proto/proto.h"
#include "proto/timestamp.h"
#include "utils/copymove.h"

class CompositeAddMessageRequest;
class CompositePingRequest;

class MasterNode : public std::enable_shared_from_this<MasterNode>, private logger::Entity<MasterNode>
{
    friend class CompositeAddMessageRequest;
    friend class CompositePingRequest;
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

    void run();

    /**
     * @param iContext     - specifies io context to perform asynchronous operations in
     * @param writeConcern - includes master node itself
     * @note resulting future will never be populated with result if @p writeConcern is bigger than the number
     *      of connected secondary nodes
     * @note thread safe
     */
    boost::future<bool> addMessage(boost::asio::io_context &ioContext, std::string, size_t writeConcern);
    /**
     * @note thread safe
     */
    std::vector<Proto::Message> getMessages();

private:
    MasterNode(boost::asio::io_context &, unsigned short, std::shared_ptr<NetUtils::IOContextPool>);

    enum class SecondaryStatus
    {
        Registering,
        Active
    };

    struct Secondary
    {
        DISABLE_COPY_MOVE(Secondary)

        Secondary() = default;

        size_t                                              id{};
        SecondaryStatus                                     status{ SecondaryStatus::Registering };
        std::shared_ptr<Proto::CommunicationEndpoint>       endpoint{};
        std::unique_ptr<NetUtils::IOContextPool::LoadGuard> loadGuard{};
    };

    size_t getNextSecondaryId();
    void   addSecondary(boost::asio::ip::tcp::socket, std::unique_ptr<NetUtils::IOContextPool::LoadGuard>);
    void   markRegistered(size_t secondaryId, std::shared_ptr<boost::promise<Proto::Response::SecondaryNodeReady>>);
    void   removeSecondary(size_t secondaryId);

    tbb::concurrent_unordered_map<Proto::Timestamp_t, std::string> messages_;

    boost::asio::io_context                  &ioContext_;
    std::shared_ptr<NetUtils::SocketAcceptor> acceptor_;

    std::atomic<size_t>                                    secondaryCounter_;
    std::shared_mutex                                      secondariesMutex_;
    std::unordered_map<size_t, std::unique_ptr<Secondary>> secondaries_;
};
