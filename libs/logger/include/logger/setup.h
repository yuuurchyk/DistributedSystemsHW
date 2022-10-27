#pragma once

#include <string>

namespace logger
{
// NOTE: these functions should be called only from main thread at start and end
// of the program
void setup(std::string programName);
void teardown();

}    // namespace logger
