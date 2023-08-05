#pragma once

#include <vector>

#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {

// Generates abstract machine instructions for `graph`.
//
// All statements that are reachable from `graph` must be allocated on `arena`.
std::vector<Instruction> GenerateAbstractMachineInstructions(
    const Arena<Stmt>& arena, const ControlFlowGraph& graph);

}  // namespace lucid
