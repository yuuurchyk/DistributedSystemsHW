#include "messagesstorage/messagesstorage.h"

#include <utility>

#include <tbb/concurrent_vector.h>

struct MessagesStorage::impl_t
{
    tbb::concurrent_vector<std::string> v;
};

MessagesStorage::MessagesStorage() : implJAKdnaGm_{ std::make_unique<impl_t>() } {}

MessagesStorage::~MessagesStorage() = default;

void MessagesStorage::addMessage(const std::string &s)
{
    impl().v.push_back(s);
}

void MessagesStorage::addMessage(std::string s)
{
    impl().v.push_back(std::move(s));
}

std::vector<std::string_view> MessagesStorage::getMessages() const
{
    auto res = std::vector<std::string_view>{};
    res.reserve(impl().v.size());

    for (auto it = impl().v.begin(); it != impl().v.end(); ++it)
        res.push_back(std::string_view{ *it });

    return res;
}

auto MessagesStorage::impl() -> impl_t &
{
    return *implJAKdnaGm_;
}

auto MessagesStorage::impl() const -> const impl_t &
{
    return *implJAKdnaGm_;
}
