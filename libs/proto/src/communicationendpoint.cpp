#include "proto/communicationendpoint.h"

#include <cstdint>

#include "utils/copymove.h"

/**
 * @brief Websocket frame structure:
 * EventType        1 byte
 * RequestId_t      8 bytes (request number in case of request, request id in case of
 * response) optional payload
 */
namespace
{
}    // namespace
