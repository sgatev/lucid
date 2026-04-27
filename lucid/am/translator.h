#pragma once

#include "lucid/am/cfg.h"
#include "lucid/am/state.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {

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
