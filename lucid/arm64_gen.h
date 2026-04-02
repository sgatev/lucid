#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "lucid/am/cfg.h"
#include "lucid/arm64.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM machine code.
void GenerateArmStartBinary(arm64::Assembler& Assembler);

// Generates the end sequence for 64-bit ARM machine code.
void GenerateArmEndBinary(
    const SyntaxContext& ctx,
    const std::unordered_map<std::uintptr_t, StringIndex::Ref>& strings,
    arm64::Assembler& Assembler);

// Generates 64-bit ARM machine code for `func`.
void GenerateArmAssemblyBinary(std::string_view func_name,
                               const std::vector<std::size_t>& stack_slots,
                               const AbstractMachineControlFlowGraph& am_cfg,
                               arm64::Assembler& assmebler);

}  // namespace lucid
