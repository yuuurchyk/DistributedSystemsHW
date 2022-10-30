#pragma once

#include <cstddef>

namespace constants
{
constexpr unsigned short kMasterHttpPort{ 8000 };
constexpr unsigned short kSecondaryHttpPort{ 8000 };
constexpr unsigned short kSecondaryCommunicationPort{ 6000 };

// workersNum threads + master communication thread + http acceptor thread
constexpr size_t kSecondaryWorkersNum{ 3 };

// workersNum threads + 1 thread per secondary node + http acceptor thread
constexpr size_t kMasterWorkersNum{ 3 };

constexpr size_t kHttpRequestTimeoutMs{ 5000 };
constexpr size_t kHttpRequestBufferSize{ 1024 };

}    // namespace constants
