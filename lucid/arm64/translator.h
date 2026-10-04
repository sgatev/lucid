#pragma once

#include <string_view>
#include <vector>

#include "lucid/am/abi.h"
#include "lucid/am/cfg.h"
#include "lucid/am/state.h"
#include "lucid/arm64/assembler.h"
#include "lucid/syntax/context.h"

namespace lucid {

// What this backend expects of a call: ten arguments in `x1` through `x10`,
// the rest in eight bytes each at the foot of the caller's frame, and the
// result in `x0`.
//
// Ten, because every parameter is live where a function is entered and so
// holds one of the ten colours the allocator has to hand out there.
inline constexpr CallingConvention kArm64CallingConvention = {
    .max_register_args = 10,
    .stack_arg_size = 8,
};

// Generates the start sequence for 64-bit ARM machine code.
void GenerateArmStartBinary(arm64::Assembler& assembler);

// Generates the end sequence for 64-bit ARM machine code.
void GenerateArmEndBinary(const SyntaxContext& syn_ctx,
                          const AbstractMachineState& am_state,
                          arm64::Assembler& assembler);

// Generates 64-bit ARM machine code for `func`.
void GenerateArmAssemblyBinary(std::string_view func_name,
                               const std::vector<int>& stack_slots,
                               const FrameLayout& layout,
                               const AbstractMachineControlFlowGraph& am_cfg,
                               arm64::Assembler& assmebler);

}  // namespace lucid
