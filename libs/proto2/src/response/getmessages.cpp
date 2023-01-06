#include "response/getmessages.h"

#include <algorithm>
#include <cassert>
#include <utility>

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace
{
std::vector<size_t> calcSizes(const std::vector<std::string_view> &views)
{
    auto res = std::vector<size_t>{};
    res.reserve(views.size());

    for (const auto &view : views)
        res.push_back(view.size());

    return res;
}

}    // namespace

namespace Proto2::Response
{

std::shared_ptr<GetMessages> GetMessages::create(std::vector<std::string_view> messagesView)
{
    return std::shared_ptr<GetMessages>{ new GetMessages{ std::move(messagesView) } };
}

std::shared_ptr<GetMessages> GetMessages::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    const auto optMessagesNum = deserializer.deserialize<size_t>();

    if (!optMessagesNum.has_value())
        return {};

    const auto messagesNum = optMessagesNum.value();
    auto       messages    = std::vector<std::string>{};
    messages.reserve(messagesNum);

    for (auto i = size_t{}; i < messagesNum; ++i)
    {
        const auto optMessageSize = deserializer.deserialize<size_t>();

        if (!optMessageSize.has_value())
            return {};

        const auto messageSize = optMessageSize.value();

        auto optMessage = deserializer.deserialize<std::string>(messageSize);

        if (!optMessage.has_value())
            return {};

        messages.push_back(std::move(optMessage.value()));
    }

    if (!deserializer.atEnd())
        return {};
    else
        return std::shared_ptr<GetMessages>{ new GetMessages{ std::move(messages) } };
}

void GetMessages::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serialize(&messagesNum_);

    assert(messagesNum_ == views_.size());
    assert(messagesNum_ == sizes_.size());

    const auto max = std::max(views_.size(), sizes_.size());
    for (auto i = size_t{}; i < max; ++i)
    {
        const auto &size = sizes_[i];
        const auto &view = views_[i];

        serializer.serialize(&size);
        serializer.serialize(&view);
    }
}

std::vector<std::string_view> GetMessages::messagesView() const
{
    return views_;
}

std::vector<std::string> GetMessages::flushMessages()
{
    if (!messages_.has_value())
    {
        auto messages = std::vector<std::string>{};
        messages.reserve(views_.size());

        for (const auto &view : views_)
            messages.emplace_back(view.data(), view.size());
    }

    return std::move(messages_.value());
}

const OpCode &GetMessages::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::GET_MESSAGES };
    return kOpCode;
}

GetMessages::GetMessages(std::vector<std::string_view> views)
    : views_{ std::move(views) }, messagesNum_{ views_.size() }, sizes_{ calcSizes(views_) }
{
}

GetMessages::GetMessages(std::vector<std::string> messages)
    : messages_{ std::move(messages) },
      views_{ [](const std::vector<std::string> &messages) -> std::vector<std::string_view>
              {
                  auto res = std::vector<std::string_view>{};
                  res.reserve(messages.size());

                  for (const auto &message : messages)
                      res.emplace_back(message);

                  return res;
              }(messages_.value()) },
      messagesNum_{ views_.size() },
      sizes_{ calcSizes(views_) }
{
}

}    // namespace Proto2::Response
