#include "request/addmessage.h"

#include <utility>

#include "deserialization/bufferdeserializer.h"
#include "serialization/buffersequenceserializer.h"

namespace Proto2::Request
{
std::shared_ptr<AddMessage> AddMessage::create(size_t messageId, std::string_view messageView)
{
    return std::shared_ptr<AddMessage>{ new AddMessage{ messageId, messageView } };
}

std::shared_ptr<AddMessage> AddMessage::fromPayload(boost::asio::const_buffer buffer)
{
    auto deserializer = BufferDeserializer{ buffer };

    auto optId      = deserializer.deserialize<size_t>();
    auto optMessage = deserializer.deserialize<std::string>();

    if (!optId.has_value() || !optMessage.has_value() || !deserializer.atEnd())
        return {};
    else
        return create(optId.value(), std::move(optMessage.value()));
}

void AddMessage::serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serializeWoOwnership(messageId_);
    serializer.serializeWoOwnership(messageView_);
}

size_t AddMessage::messageId() const noexcept
{
    return messageId_;
}

std::string_view AddMessage::messageView() const
{
    return messageView_;
}

std::string AddMessage::flushMessage()
{
    if (!messageOwner_.has_value())
        messageOwner_ = std::string{ messageView_ };

    return std::move(messageOwner_.value());
}

const OpCode &AddMessage::opCode() const
{
    static constexpr OpCode kOpCode{ OpCode::ADD_MESSAGE };
    return kOpCode;
}

AddMessage::AddMessage(size_t messageId, std::string_view messageView)
    : messageId_{ messageId }, messageView_{ messageView }
{
}

AddMessage::AddMessage(size_t messageId, std::string message)
    : messageId_{ messageId }, messageOwner_{ std::move(message) }, messageView_{ messageOwner_.value() }
{
}

}    // namespace Proto2::Request
