#pragma once

#include "lucid/cfg.h"
#include "lucid/syntax/ast.h"

namespace lucid {

// Prints the basic blocks in `graph`.
//
// Requires:
// - `graph` must be constructed in `ctx`.
void Print(const SyntaxContext& ctx, const ControlFlowGraph& graph);

}  // namespace lucid
