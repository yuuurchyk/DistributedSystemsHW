#pragma once

#include <memory>
#include <string>
#include <vector>

#include "utils/copymove.h"

/**
 * @brief all operations are thread safe
 */
class MessagesStorage
{
    DISABLE_COPY_MOVE(MessagesStorage);

public:
    MessagesStorage();
    ~MessagesStorage();

    void addMessage(const std::string &);
    void addMessage(std::string &&);

    std::vector<std::string_view> getMessages() const;

private:
    struct impl_t;
    std::unique_ptr<impl_t> implJAKdnaGm_{};

    impl_t       &impl();
    const impl_t &impl() const;
};
