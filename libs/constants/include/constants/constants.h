#pragma once

#include <cstddef>

namespace constants
{
constexpr unsigned short kMasterHttpPort{ 8000 };

constexpr unsigned short kSecondary1HttpPort{ 7000 };
constexpr unsigned short kSecondary1CommunicationPort{ 6000 };

constexpr unsigned short kSecondary2HttpPort{ 7001 };
constexpr unsigned short kSecondary2CommunicationPort{ 6001 };

// workersNum threads + master communication thread + http acceptor thread
constexpr size_t kSecondaryWorkersNum{ 3 };

// workersNum threads + 1 thread per secondary node + http acceptor thread
constexpr size_t kMasterWorkersNum{ 3 };
// how often do we try to reconnect to secondary node
constexpr size_t kSecondaryReconnectTimeoutMs{ 3000 };

constexpr size_t kProtocolFrameTimeoutMs{ 1500 };

constexpr size_t kHttpRequestTimeoutMs{ 5000 };
constexpr size_t kHttpRequestBufferSize{ 1024 };

}    // namespace constants
