#pragma once

#include <ostream>
#include <string_view>
#include <vector>

#include "lucid/am.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM assembly source code and writes
// it to `out`.
void GenerateArmStartSource(std::ostream& out);

// Generates 64-bit ARM assembly source code for `func` and writes it to `out`.
void GenerateArmAssemblySource(std::string_view func_name, const Function& func,
                               std::ostream& out);

}  // namespace lucid
