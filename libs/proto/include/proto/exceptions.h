#pragma once

#include <exception>

namespace Proto::Exceptions
{
struct ResponseException : std::exception
{
};

struct BadResponseFrame : ResponseException
{
    const char *what() const noexcept override { return "Bad response frame format"; }
};

struct Timeout : ResponseException
{
    const char *what() const noexcept override { return "Timeout occured for request"; }
};

struct PeerDisconnected : ResponseException
{
    const char *what() const noexcept override { return "Peer disconnected"; }
};

}    // namespace Proto::Exceptions
