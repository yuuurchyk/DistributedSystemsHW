#pragma once

#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "logger/logger.h"
#include "proto/proto.h"
#include "utils/copymove.h"

namespace Proto
{
/**
 * @brief websocket implementation of messages exchange system
 *
 */
class CommunicationEndpoint : public std::enable_shared_from_this<CommunicationEndpoint>,
                              public logger::StringIdEntity<CommunicationEndpoint>
{
    DISABLE_COPY_MOVE(CommunicationEndpoint)
public:
    [[nodiscard]] static std::shared_ptr<CommunicationEndpoint>
        create(boost::asio::io_context &, boost::asio::ip::tcp::socket);

    // thread safe
    boost::future<Response::AddMessage>  addMessage(Request::AddMessage);
    boost::future<Response::GetMessages> getMessages(Request::GetMessages);

public:    // signals
    // clang-format off
    template <typename T>
    using signal = boost::signals2::signal<T>;

    signal<void()> invalidated;

    signal<void(Request::AddMessage  &, std::shared_ptr<boost::promise<Response::AddMessage>>)>   requestAddMessages;
    signal<void(Request::GetMessages &, std::shared_ptr<boost::promise<Response::GetMessages>>)> requestGetMessages;
    // clang-format on

private:
    using RequestId_t = uint64_t;
    RequestId_t getNextRequestId();

    void invalidate();

private:
    boost::asio::io_context     &context_;
    boost::asio::ip::tcp::socket socket_;

    std::atomic<RequestId_t> requestIdCounter_;
};

}    // namespace Proto
