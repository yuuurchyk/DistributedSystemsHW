#include "compositeaddmessagerequest.h"

#include <utility>

std::shared_ptr<CompositeAddMessageRequest> CompositeAddMessageRequest::create(
    boost::asio::io_context  &ioContext,
    std::weak_ptr<MasterNode> weakMasterNode,
    Proto::Message            message,
    size_t                    writeConcern)
{
    return std::shared_ptr<CompositeAddMessageRequest>{ new CompositeAddMessageRequest{
        ioContext, std::move(weakMasterNode), std::move(message), writeConcern } };
}

CompositeAddMessageRequest::~CompositeAddMessageRequest()
{
    EN_LOGI << "~CompositeAddMessageRequest()";
    waitTimer_.cancel();
}

boost::future<bool> CompositeAddMessageRequest::getFuture()
{
    return promise_.get_future();
}

void CompositeAddMessageRequest::run()
{
    boost::asio::post(ioContext_, [this, self = shared_from_this()]() { sendRequestToSecondaries(); });
}

CompositeAddMessageRequest::CompositeAddMessageRequest(
    boost::asio::io_context  &ioContext,
    std::weak_ptr<MasterNode> weakMasterNode,
    Proto::Message            message,
    size_t                    writeConcern)
    : logger::StringIdEntity<CompositeAddMessageRequest>{ message.message },
      ioContext_{ ioContext },
      weakMasterNode_{ std::move(weakMasterNode) },
      message_{ std::move(message) },
      waitTimer_{ ioContext },
      writeConcern_{ writeConcern }
{
    if (writeConcern_ == 0)
    {
        EN_LOGI << "No confirmations from secondaries are necessary, setting response right away";
        setPromiseValue();
    }
}

void CompositeAddMessageRequest::sendRequestToSecondaries()
{
    EN_LOGI << "sendRequestToSecondaries()";

    auto masterNode = lockMaster();
    if (masterNode == nullptr)
        return;

    std::shared_lock<std::shared_mutex> lck{ masterNode->secondariesMutex_ };

    pending_.clear();
    cnt_ = 0;

    for (auto successesIt = successes_.begin(); successesIt != successes_.end();)
    {
        const auto &secondaryId = successesIt->first;
        auto       &status      = successesIt->second;

        const auto it = masterNode->secondaries_.find(secondaryId);

        if (it == masterNode->secondaries_.end())
        {
            successesIt = successes_.erase(successesIt);
            continue;
        }
        else
        {
            // update status from master node
            status = it->second->status;
            if (status == MasterNode::SecondaryStatus::Active)
                ++cnt_;
            ++successesIt;
        }
    }

    if (cnt_ >= writeConcern_ && !promiseSet_)
        setPromiseValue();

    for (const auto &[secondaryId, secondaryNode] : masterNode->secondaries_)
    {
        const auto it = successes_.find(secondaryId);

        if (it != successes_.end())
            continue;

        pending_.insert({ secondaryId, secondaryNode->status });

        secondaryNode->endpoint
            ->send_addMessage(std::make_shared<Proto::Request::AddMessage>(message_.timestamp, message_.message))
            .then(
                [this, secondaryId, self = shared_from_this()](
                    boost::future<Proto::Response::AddMessage> responseFuture) mutable
                {
                    boost::asio::post(
                        ioContext_,
                        [this,
                         secondaryId,
                         responseFuture = std::move(responseFuture),
                         self           = std::move(self)]() mutable
                        { onResponseRecieved(secondaryId, std::move(responseFuture)); });
                });
    }

    if (!promiseSet_ && pending_.empty())
    {
        EN_LOGI << "no pending secondary nodes present, but write concern is not met, waiting";
        waitTimer_.expires_from_now(kDelay);
        waitTimer_.async_wait(
            [this, self = shared_from_this()](const boost::system::error_code &ec)
            {
                if (ec)
                    return;

                sendRequestToSecondaries();
            });
    }
}

void CompositeAddMessageRequest::setPromiseValue()
{
    if (promiseSet_)
        return;

    EN_LOGI << "Setting promise to true";

    promiseSet_ = true;
    promise_.set_value(true);
}

void CompositeAddMessageRequest::onResponseRecieved(
    size_t                                     secondaryId,
    boost::future<Proto::Response::AddMessage> responseFuture)
{
    EN_LOGI << "Response recieved from secondary node, id=" << secondaryId;

    auto nodeState = MasterNode::SecondaryStatus::Registering;

    if (auto it = pending_.find(secondaryId); it != pending_.end())
    {
        nodeState = it->second;
        pending_.erase(it);
    }

    if (responseFuture.has_value() && responseFuture.get().status == Proto::Response::AddMessage::Status::OK)
    {
        EN_LOGI << "Response ok (secondary node, id=" << secondaryId << ")";
        successes_.insert({ secondaryId, nodeState });

        if (nodeState == MasterNode::SecondaryStatus::Active)
        {
            EN_LOGI << "Decreasing write concern";
            ++cnt_;
            if (cnt_ >= writeConcern_)
                setPromiseValue();
        }
    }
    else
    {
        EN_LOGI << "Response not ok (secondary node, id=" << secondaryId << ")";
    }

    if (pending_.empty())
        sendRequestToSecondaries();
}

std::shared_ptr<MasterNode> CompositeAddMessageRequest::lockMaster()
{
    if (invalidated_)
        return nullptr;

    auto res = weakMasterNode_.lock();
    if (res == nullptr)
    {
        EN_LOGW << "Failed to lock master node, invalidating";
        invalidated_ = true;
        if (!promiseSet_)
        {
            promiseSet_ = true;
            promise_.set_value(false);
        }
    }

    return res;
}
