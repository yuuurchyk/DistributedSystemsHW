#include "compositepingrequest.h"

#include <utility>

#include <boost/json.hpp>

std::shared_ptr<CompositePingRequest>
    CompositePingRequest::create(boost::asio::io_context &ioContext, std::weak_ptr<MasterNode> weakMasterNode)
{
    return std::shared_ptr<CompositePingRequest>{ new CompositePingRequest{ ioContext, std::move(weakMasterNode) } };
}

boost::future<std::optional<std::string>> CompositePingRequest::getFuture()
{
    return promise_.get_future();
}

void CompositePingRequest::run()
{
    boost::asio::post(ioContext_, [this, self = shared_from_this()]() { sendRequestToSecondaries(); });
}

CompositePingRequest::CompositePingRequest(boost::asio::io_context &ioContext, std::weak_ptr<MasterNode> weakMasterNode)
    : ioContext_{ ioContext }, weakMasterNode_{ std::move(weakMasterNode) }
{
}

void CompositePingRequest::sendRequestToSecondaries()
{
    auto masterNode = weakMasterNode_.lock();
    if (masterNode == nullptr)
    {
        EN_LOGW << "Failed to find master node, aborting";
        promise_.set_value({});
        return;
    }

    std::shared_lock<std::shared_mutex> lck{ masterNode->secondariesMutex_ };

    EN_LOGI << "Sending ping request to secondaries, fixing timestamp";
    sendTimestamp_ = Proto::getCurrentTimestamp();

    ids_.reserve(masterNode->secondaries_.size());
    statuses_.reserve(masterNode->secondaries_.size());
    futures_.reserve(masterNode->secondaries_.size());
    for (auto &[id, secondary] : masterNode->secondaries_)
    {
        ids_.push_back(id);
        statuses_.push_back(secondary->status);
        futures_.push_back(secondary->endpoint->send_ping(std::make_shared<Proto::Request::Ping>(sendTimestamp_)));
    }

    boost::when_all(futures_.begin(), futures_.end())
        .then(
            [this, self = shared_from_this()](
                boost::future<std::vector<boost::future<Proto::Response::Pong>>> responsesFuture)
            {
                boost::asio::post(
                    ioContext_,
                    [this, responsesFuture = std::move(responsesFuture), self]() mutable
                    { onAllResponsesRecieved(std::move(responsesFuture)); });
            });
}

void CompositePingRequest::onAllResponsesRecieved(
    boost::future<std::vector<boost::future<Proto::Response::Pong>>> responsesFuture)
{
    if (!responsesFuture.has_value())
    {
        EN_LOGW << "Failed to recieve response";
        return promise_.set_value({});
    }

    EN_LOGI << "Recieved responses";

    auto responses = responsesFuture.get();

    const auto n   = responses.size();
    auto       res = boost::json::array{};
    res.reserve(n);

    for (size_t i = 0; i < n; ++i)
    {
        const auto id     = ids_[i];
        const auto status = statuses_[i];

        auto &responseFuture = responses[i];

        auto obj  = boost::json::object{};
        obj["id"] = id;

        switch (status)
        {
        case MasterNode::SecondaryStatus::Active:
            obj["status"] = "Active";
            break;
        case MasterNode::SecondaryStatus::Registering:
            obj["status"] = "Registering";
            break;
        default:
            obj["status"] = "UNKNOWN";
            break;
        }

        try
        {
            obj["pongTimestamp"] = responseFuture.get().timestamp;
        }
        catch (const Proto::DisconnectedException &)
        {
            obj["pongTimestamp"] = "ERROR: DISCONNECTED";
        }
        catch (const Proto::TimeoutException &)
        {
            obj["pongTimestamp"] = "ERROR: TIMEOUT";
        }
        catch (const std::exception &)
        {
            obj["pongTimestamp"] = "UNKNOWN ERROR";
        }

        res.push_back(std::move(obj));
    }

    auto ans             = boost::json::object{};
    ans["pingTimestamp"] = sendTimestamp_;
    ans["repsonses"]     = std::move(res);

    promise_.set_value(boost::json::serialize(ans));
}
