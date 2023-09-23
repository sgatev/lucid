#pragma once

#include <vector>

#include "lucid/am.h"
#include "lucid/arena.h"
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
};

// Generates abstract machine instructions for `graph`.
//
// The generated instructions are stored in `state`.
//
// All statements that are reachable from `graph` must be allocated on `arena`.
void GenerateAbstractMachineFunction(const Arena<Stmt>& arena,
                                     const ControlFlowGraph& graph,
                                     AbstractMachineState& state);

}  // namespace lucid
