#pragma once

#include <cstdint>

#include "lucid/am/cfg.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Abstract machine stack slots.
  std::vector<std::size_t> stack_slots;

  // Strings used in `func`.
  HashMap<std::uintptr_t, StringIndex::Ref> strings;

  // Next free register ID.
  std::int32_t next_free_reg_id = 1;
};

// Generates abstract machine instructions for `scfg`.
//
// The generated instructions are stored in `state`.
//
// Requires:
// - `scfg` must be constructed in `ctx`.
AbstractMachineControlFlowGraph GenerateAbstractMachineFunction(
    const SyntaxContext& ctx, const SyntaxControlFlowGraph& scfg,
    AbstractMachineState& state);

}  // namespace lucid
