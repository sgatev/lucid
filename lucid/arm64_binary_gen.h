#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>

#include "lucid/am.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM machine code and writes it to
// `out`.
void GenerateArmStartBinary(std::ostream& out);

// Generates the end sequence for 64-bit ARM machine code and writes it to
// `out`.
void GenerateArmEndBinary(
    const std::unordered_map<std::uintptr_t, std::string>& strings,
    std::ostream& out);

// Generates 64-bit ARM machine code for `func` and writes it to `out`.
void GenerateArmAssemblyBinary(const Function& func, std::ostream& out);

}  // namespace lucid
