#include "mastersession/mastersession.h"

#include <utility>

#include "constants2/constants2.h"
#include "net-utils/thenpost.h"

std::shared_ptr<MasterSession> MasterSession::create(
    std::string                  friendlyName,
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    Storage                     &storage)
{
    auto self = std::shared_ptr<MasterSession>{ new MasterSession{
        std::move(friendlyName), ioContext, std::move(socket), storage } };

    self->establishConnections();

    return self;
}

void MasterSession::run()
{
    endpoint_->run();
    askForMessages();
}

MasterSession::MasterSession(
    std::string                  friendlyName,
    boost::asio::io_context     &ioContext,
    boost::asio::ip::tcp::socket socket,
    Storage                     &storage)
    : friendlyName_{ std::move(friendlyName) },
      ioContext_{ ioContext },
      endpoint_{ Proto2::Endpoint::create(
          "master",
          ioContext,
          std::move(socket),
          Constants2::kOutcomingRequestTimeout,
          Constants2::kArtificialSendDelayBounds) },
      storage_{ storage }
{
}

void MasterSession::establishConnections()
{
    endpoint_->invalidated.connect(
        [this]()
        {
            EN_LOGW << "socket invalidated";
            invalidated();
        });

    endpoint_->incoming_addMessage.connect(
        [this](size_t msgId, std::string msg, Proto2::SharedPromise<Proto2::AddMessageStatus> response)
        {
            storage_.addMessage(msgId, std::move(msg));
            response->set_value(Proto2::AddMessageStatus::OK);
        });
    endpoint_->incoming_ping.connect([this](Proto2::Timestamp_t, Proto2::SharedPromise<Proto2::Timestamp_t> response)
                                     { response->set_value(Proto2::getCurrentTimestamp()); });
}

void MasterSession::askForMessages()
{
    const auto gap = storage_.getFirstGap();

    EN_LOGI << "asking master for messages, starting from messageId=" << gap;

    NetUtils::thenPost(
        endpoint_->send_getMessages(gap),
        ioContext_,
        [this, gap, weakSelf = weak_from_this()](boost::future<std::vector<std::string>> response)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (response.has_value())
            {
                EN_LOGI << "Successfully retrieved messages from master, filling storage";
                storage_.addMessages(gap, response.get());
                operational();
                notifyOperational();
            }
            else
            {
                EN_LOGW << "Failed to recieve messages from master, retrying";
                askForMessages();
            }
        });
}

void MasterSession::notifyOperational()
{
    EN_LOGI << "sending friendly name to master";

    NetUtils::thenPost(
        endpoint_->send_secondaryNodeReady(friendlyName_),
        ioContext_,
        [this, weakSelf = weak_from_this()](boost::future<void> response)
        {
            const auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            if (response.has_value())
            {
                EN_LOGI << "recieved ack from master on secondaryNodeReady request";
            }
            else
            {
                EN_LOGW << "failed to recieve ack from master on secondaryNodeReady request, retrying";
                notifyOperational();
            }
        });
}
