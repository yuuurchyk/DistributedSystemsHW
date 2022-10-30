#pragma once

#include <cstddef>

namespace constants
{
constexpr unsigned short kMasterHttpPort{ 8000 };
constexpr unsigned short kSecondaryHttpPort{ 8000 };
constexpr unsigned short kSecondaryCommunicationPort{ 6000 };

// workersNum threads + master communication thread + http acceptor thread
constexpr size_t kSecondaryWorkersNum{ 3 };

}    // namespace constants
