#pragma once

#include <cstdint>
#include <unordered_map>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Generated abstract machine function after the last call to
  // `GenerateAbstractMachineFunction` where this state was used.
  Function func;

  // Strings used in `func`.
  std::unordered_map<std::uintptr_t, StringIndex::Ref> strings;
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
