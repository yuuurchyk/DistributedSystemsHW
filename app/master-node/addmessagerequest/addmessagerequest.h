#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "proto2/enums.h"
#include "utils/copymove.h"

#include "masternode/masternode.h"
#include "secondary/secondarysnapshot.h"

class AddMessageRequest : public std::enable_shared_from_this<AddMessageRequest>,
                          private logger::NumIdEntity<AddMessageRequest>
{
    DISABLE_COPY_MOVE(AddMessageRequest)
public:
    [[nodiscard]] static std::shared_ptr<AddMessageRequest> create(
        boost::asio::io_context  &executionContext,
        std::weak_ptr<MasterNode> masterNode,
        size_t                    messageId,
        std::string_view          message,
        size_t                    writeConcern);

    boost::future<void> future();

    void run();

private:
    AddMessageRequest(
        boost::asio::io_context &,
        std::weak_ptr<MasterNode>,
        size_t           messageId,
        std::string_view message,
        size_t           writeConcern);

    [[nodiscard]] size_t satisfiedWriteConcern() const;
    [[nodiscard]] bool   allSecondariesSuccessfullyAnswered() const;

    void start();

    bool updateData();
    void sendRequestToSecondaries();

    void requestMarkSuccess();
    void requestMarkFailure();

    void onResponseRecieved(size_t secondaryId, boost::future<Proto2::AddMessageStatus> response);
    void onAllResponsesRecieved();

private:
    boost::asio::io_context        &ioContext_;
    const std::weak_ptr<MasterNode> masterNode_;

    const size_t writeConcern_;

    const size_t           messageId_;
    const std::string_view message_;

    bool                 promiseFilled_{};
    boost::promise<void> promise_;

    enum class Status
    {
        ADDED,
        NOT_ADDED,
        ERROR
    };

    size_t pendingWrites_{};

    std::vector<SecondarySnapshot>                       secondariesSnapshot_;
    std::unordered_map<size_t /* secondaryId */, Status> statuses_;
};
