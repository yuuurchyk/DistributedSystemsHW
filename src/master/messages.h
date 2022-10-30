#pragma once

#include <string>

#include <tbb/concurrent_vector.h>

extern tbb::concurrent_vector<std::string> messages;
