#pragma once

#include <ostream>
#include <string_view>
#include <vector>

#include "lucid/am.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM assembly source code and writes
// it to `out`.
void GenerateArmStartSource(std::ostream& out);

// Generates 64-bit ARM assembly source code for a function named `func_name`
// with a set of `instructions` and writes it to `out`.
void GenerateArmAssemblySource(std::string_view func_name,
                               const std::vector<Instruction>& instructions,
                               std::ostream& out);

}  // namespace lucid
