#include "addmessagerequest/addmessagerequest.h"

#include <exception>
#include <stdexcept>
#include <unordered_set>
#include <utility>

#include "constants2/constants2.h"
#include "net-utils/launchwithdelay.h"
#include "net-utils/thenpost.h"
#include "utils/duration.h"

std::shared_ptr<AddMessageRequest> AddMessageRequest::create(
    boost::asio::io_context  &ioContext,
    std::weak_ptr<MasterNode> masterNode,
    size_t                    messageId,
    std::string_view          message,
    size_t                    writeConcern)
{
    return std::shared_ptr<AddMessageRequest>{ new AddMessageRequest{
        ioContext, std::move(masterNode), messageId, std::move(message), writeConcern } };
}

AddMessageRequest::AddMessageRequest(
    boost::asio::io_context  &ioContext,
    std::weak_ptr<MasterNode> masterNode,
    size_t                    messageId,
    std::string_view          message,
    size_t                    writeConcern)
    : ioContext_{ ioContext },
      masterNode_{ std::move(masterNode) },
      writeConcern_{ writeConcern },
      messageId_{ messageId },
      message_{ std::move(message) }
{
}

boost::future<void> AddMessageRequest::future()
{
    return promise_.get_future();
}

void AddMessageRequest::run()
{
    start();
}

void AddMessageRequest::start()
{
    if (!updateData())
    {
        EN_LOGW << "updateData failed";
        requestMarkFailure();
        return;
    }

    if (satisfiedWriteConcern() >= writeConcern_)
    {
        requestMarkSuccess();

        if (allSecondariesSuccessfullyAnswered())
        {
            EN_LOGI << "all secondaries answered, destroying request";
            return;
        }
    }

    sendRequestToSecondaries();
}

bool AddMessageRequest::updateData()
{
    const auto masterNode = masterNode_.lock();
    if (masterNode == nullptr)
        return false;

    secondariesSnapshot_ = masterNode->secondariesSnapshot();

    {
        auto presentSecondaries = std::unordered_set<int>{};
        presentSecondaries.reserve(secondariesSnapshot_.size());

        for (auto &secondary : secondariesSnapshot_)
            presentSecondaries.insert(secondary.id);

        auto it = statuses_.begin();
        while (it != statuses_.end())
        {
            if (presentSecondaries.contains(it->first))
                ++it;
            else
                it = statuses_.erase(it);
        }
    }

    return true;
}

void AddMessageRequest::sendRequestToSecondaries()
{
    if (pendingWrites_ > 0)
    {
        EN_LOGE << "assumed 0 pending writes, quitting";
        return;
    }

    for (auto &secondary : secondariesSnapshot_)
    {
        bool needToSend = true;

        if (statuses_.contains(secondary.id) && statuses_[secondary.id] != Status::ERROR)
            needToSend = false;

        if (!needToSend)
            continue;

        ++pendingWrites_;

        NetUtils::thenPost(
            secondary.endpoint->send_addMessage(messageId_, message_),
            ioContext_,
            [this, secondaryId = secondary.id, self = shared_from_this()](
                boost::future<Proto::AddMessageStatus> future)
            {
                onResponseRecieved(secondaryId, std::move(future));

                --pendingWrites_;
                if (pendingWrites_ == 0)
                    onAllResponsesRecieved();
            });
    }

    if (pendingWrites_ == 0)
    {
        EN_LOGI << "write concern is not satisfied, but there is not secondaries to query, waiting...";
        NetUtils::launchWithDelay(
            ioContext_,
            Utils::toPosixTime(Constants2::kMasterReconnectTimeout),
            [this, self = shared_from_this()]() { start(); });
    }
}

void AddMessageRequest::requestMarkSuccess()
{
    if (promiseFilled_)
        return;

    EN_LOGI << "marking request as success";

    promiseFilled_ = true;
    promise_.set_value();
}

void AddMessageRequest::requestMarkFailure()
{
    if (promiseFilled_)
        return;

    EN_LOGI << "marking request as failure";

    promiseFilled_ = true;

    try
    {
        throw std::logic_error{ "internal request error" };
    }
    catch (const std::exception &e)
    {
        promise_.set_exception(std::current_exception());
    }
}

void AddMessageRequest::onResponseRecieved(size_t secondaryId, boost::future<Proto::AddMessageStatus> response)
{
    auto &status = statuses_[secondaryId];

    if (response.has_value())
    {
        switch (response.get())
        {
        case Proto::AddMessageStatus::OK:
            EN_LOGI << "Recieved ok response from secondary node " << secondaryId;
            status = Status::ADDED;
            break;
        case Proto::AddMessageStatus::NOT_ALLOWED:
            EN_LOGW << "Recieved not allowed response from secondary node " << secondaryId;
            status = Status::NOT_ADDED;
            break;
        }
    }
    else
    {
        EN_LOGW << "Recieved error from secondary node " << secondaryId;
        status = Status::ERROR;
    }

    if (satisfiedWriteConcern() >= writeConcern_)
        requestMarkSuccess();
}

void AddMessageRequest::onAllResponsesRecieved()
{
    start();
}

size_t AddMessageRequest::satisfiedWriteConcern() const
{
    auto res = 0;

    for (const auto &[_, status] : statuses_)
        if (status == Status::ADDED)
            ++res;

    return res;
}

bool AddMessageRequest::allSecondariesSuccessfullyAnswered() const
{
    auto successfulAnswers = size_t{ 0 };

    for (const auto &[_, status] : statuses_)
    {
        if (status != Status::ERROR)
            ++successfulAnswers;
    }

    return successfulAnswers == secondariesSnapshot_.size();
}
