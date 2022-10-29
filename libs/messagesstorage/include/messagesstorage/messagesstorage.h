#pragma once

#include <memory>
#include <string>
#include <vector>

/**
 * @brief all operations are thread safe
 */
class MessagesStorage
{
public:
    MessagesStorage();

    MessagesStorage(const MessagesStorage &)            = delete;
    MessagesStorage(MessagesStorage &&)                 = delete;
    MessagesStorage &operator=(const MessagesStorage &) = delete;
    MessagesStorage &operator=(MessagesStorage &&)      = delete;

    ~MessagesStorage();

    void addMessage(const std::string &);
    void addMessage(std::string);

    std::vector<std::string_view> getMessages() const;

private:
    struct impl_t;
    std::unique_ptr<impl_t> implJAKdnaGm_{};

    impl_t       &impl();
    const impl_t &impl() const;
};
