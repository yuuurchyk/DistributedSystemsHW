#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "response/abstractresponse.h"

namespace Proto::Response
{
class GetMessages final : public AbstractResponse
{
public:
    [[nodiscard]] static std::shared_ptr<GetMessages> create(std::vector<std::string_view>);

    [[nodiscard]] static std::shared_ptr<GetMessages> fromPayload(boost::asio::const_buffer);
    void serializePayloadWoOwnership(std::vector<boost::asio::const_buffer> &) const override;

    std::vector<std::string_view> messagesView() const;
    // in case fromPayload() is used
    std::vector<std::string> flushMessages();

    const OpCode &opCode() const override;

private:
    GetMessages(std::vector<std::string_view>);
    GetMessages(std::vector<std::string>);

    // in case fromPayload() is used
    std::optional<std::vector<std::string>> messages_;
    const std::vector<std::string_view>     views_;

    const size_t messagesNum_{};

    const std::vector<size_t> sizes_;
};

}    // namespace Proto::Response
