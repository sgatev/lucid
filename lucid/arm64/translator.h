#pragma once

#include <cstdint>
#include <string_view>

#include "lucid/am/cfg.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM machine code.
void GenerateArmStartBinary(arm64::Assembler& Assembler);

// Generates the end sequence for 64-bit ARM machine code.
void GenerateArmEndBinary(
    const SyntaxContext& ctx,
    const HashMap<std::uintptr_t, StringIndex::Ref>& strings,
    arm64::Assembler& Assembler);

// Generates 64-bit ARM machine code for `func`.
void GenerateArmAssemblyBinary(std::string_view func_name,
                               const std::vector<std::size_t>& stack_slots,
                               const AbstractMachineControlFlowGraph& amcfg,
                               arm64::Assembler& assmebler);

}  // namespace lucid
