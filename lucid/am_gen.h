#pragma once

#include <cstdint>
#include <unordered_map>

#include "lucid/am.h"
#include "lucid/am_cfg.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/string_index.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Generated abstract machine function after the last call to
  // `GenerateAbstractMachineFunction` where this state was used.
  Function func;

  // Strings used in `func`.
  std::unordered_map<std::uintptr_t, StringIndex::Ref>& strings;
};

// Generates abstract machine instructions for `graph`.
//
// The generated instructions are stored in `state`.
//
// Requires:
// - `graph` must be constructed in `ctx`.
AbstractMachineControlFlowGraph GenerateAbstractMachineFunction(
    const SyntaxContext& ctx, const ControlFlowGraph& graph,
    AbstractMachineState& state);

}  // namespace lucid
