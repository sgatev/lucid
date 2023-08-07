#pragma once

#include <string_view>
#include <vector>

#include "lucid/am.h"
#include "lucid/writer.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM assembly source code and writes
// it using `output`.
void GenerateArmStartSource(Writer output);

// Generates 64-bit ARM assembly source code for a function named `func_name`
// with a set of `instructions` and writes it using `output`.
void GenerateArmAssemblySource(std::string_view func_name,
                               const std::vector<Instruction>& instructions,
                               Writer output);

}  // namespace lucid
