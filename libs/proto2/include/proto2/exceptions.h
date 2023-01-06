#pragma once

#include <exception>

namespace Proto2
{
struct ResponseException : std::exception
{
};

struct BadFrameException : ResponseException
{
    const char *what() const noexcept { return "Bad response frame format"; }
};

struct TimeoutException : ResponseException
{
    const char *what() const noexcept { return "Timeout occured for request"; }
};

struct DisconnectedException : ResponseException
{
    const char *what() const noexcept { return "Peer disconnected"; }
};

}    // namespace Proto2