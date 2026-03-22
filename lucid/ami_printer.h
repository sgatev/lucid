#pragma once

#include <string_view>
#include <vector>

#include "lucid/am.h"

namespace lucid {

// Prints the abstract machine instructions generated for the given function.
void Print(std::string_view func_name,
           const std::vector<Instruction>& instructions);

}  // namespace lucid
