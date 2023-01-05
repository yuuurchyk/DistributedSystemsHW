#include "proto2/request/addmessage.h"

#include "serialization/buffersequenceserializer.h"

#include <utility>

namespace Proto2::Request
{
std::unique_ptr<AddMessage> AddMessage::create(size_t messageId, std::string_view messageView)
{
    return std::unique_ptr<AddMessage>{ new AddMessage{ messageId, messageView } };
}

std::unique_ptr<AddMessage> AddMessage::fromPayload(boost::asio::const_buffer buffer)
{
    if (buffer.size() < sizeof(size_t) || buffer.data() == nullptr)
        return {};

    auto data = static_cast<const std::byte *>(buffer.data());

    auto id = size_t{};
    id      = *reinterpret_cast<const size_t *>(data);
    data += sizeof(size_t);

    auto size = buffer.size() - sizeof(size_t);
    static_assert(sizeof(char) == 1);
    auto message = std::string{ reinterpret_cast<const char *>(data), size };

    return std::unique_ptr<AddMessage>{ new AddMessage{ id, std::move(message) } };
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

void AddMessage::serializePayload(std::vector<boost::asio::const_buffer> &seq) const
{
    auto serializer = BufferSequenceSerializer{ seq };

    serializer.serialize(&messageId_);
    serializer.serialize(&messageView_);
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
