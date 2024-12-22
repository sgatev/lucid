#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "lucid/am.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Generated abstract machine function after the last call to
  // `GenerateAbstractMachineFunction` where this state was used.
  Function func;

  // Register allocation data structure that is used to generate abstract
  // machine instructions.
  std::vector<RegId> out_reg;

  // Strings used in `func`.
  std::unordered_map<std::uintptr_t, std::string> strings;
};

// Generates abstract machine instructions for `graph`.
//
// The generated instructions are stored in `state`.
//
// Requires:
// - `graph` must be constructed in `ctx`.
void GenerateAbstractMachineFunction(const SyntaxContext& ctx,
                                     const ControlFlowGraph& graph,
                                     AbstractMachineState& state);

}  // namespace lucid
