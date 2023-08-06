#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "lucid/am.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM assembly source code.
std::string GenerateArmStartSource();

// Generates 64-bit ARM assembly source code for a function named `func_name`
// with a set of `instructions`.
std::string GenerateArmAssemblySource(
    std::string_view func_name, const std::vector<Instruction>& instructions);

}  // namespace lucid
