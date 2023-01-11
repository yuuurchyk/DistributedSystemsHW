#pragma once

#include <exception>

namespace Proto2
{
struct ResponseException : std::exception
{
};

struct BadFrameException : ResponseException
{
    const char *what() const noexcept override { return "Bad response frame format"; }
};

struct TimeoutException : ResponseException
{
    const char *what() const noexcept override { return "Timeout occured for request"; }
};

struct PeerDisconnectedException : ResponseException
{
    const char *what() const noexcept override { return "Peer disconnected"; }
};

}    // namespace Proto2
