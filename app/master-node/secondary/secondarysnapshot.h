#pragma once

#include <optional>
#include <string>

#include "proto/endpoint.h"

#include "secondarystate.h"

struct SecondarySnapshot
{
    size_t                     id;
    std::optional<std::string> friendlyName;

    SecondaryState                   state;
    std::shared_ptr<Proto::Endpoint> endpoint;
};
