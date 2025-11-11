#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "lucid/am.h"
#include "lucid/arm64.h"
#include "lucid/ast.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM machine code.
void GenerateArmStartBinary(arm64::Assembler& Assembler);

// Generates the end sequence for 64-bit ARM machine code.
void GenerateArmEndBinary(
    const std::unordered_map<std::uintptr_t, std::string>& strings,
    arm64::Assembler& Assembler);

// Generates 64-bit ARM machine code for `func`.
void GenerateArmAssemblyBinary(const SyntaxContext& ctx, const Function& func,
                               arm64::Assembler& assembler);

}  // namespace lucid
