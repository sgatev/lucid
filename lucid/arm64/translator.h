#pragma once

#include <cstdint>
#include <string_view>

#include "lucid/am/cfg.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/context.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM machine code.
void GenerateArmStartBinary(arm64::Assembler& assembler);

// Generates the end sequence for 64-bit ARM machine code.
void GenerateArmEndBinary(
    const SyntaxContext& syn_ctx,
    const HashMap<std::uintptr_t, StringIndex::Ref>& strings,
    arm64::Assembler& assembler);

// Generates 64-bit ARM machine code for `func`.
void GenerateArmAssemblyBinary(std::string_view func_name,
                               const std::vector<std::size_t>& stack_slots,
                               const AbstractMachineControlFlowGraph& am_cfg,
                               arm64::Assembler& assmebler);

}  // namespace lucid
