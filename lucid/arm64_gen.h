#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>

#include "lucid/am.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM assembly source code and writes
// it to `out`.
void GenerateArmStartSource(std::ostream& out);

// Generates the end sequence for 64-bit ARM assembly source code and writes
// it to `out`.
void GenerateArmEndSource(
    const std::unordered_map<std::uintptr_t, std::string>& strings,
    std::ostream& out);

// Generates 64-bit ARM assembly source code for `func` and writes it to `out`.
void GenerateArmAssemblySource(const Function& func, std::ostream& out);

}  // namespace lucid
