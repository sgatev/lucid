#pragma once

#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {

// Prints the basic blocks in `scfg`.
//
// Requires:
// - `scfg` must be constructed in `ctx`.
void Print(const SyntaxContext& ctx, const SyntaxControlFlowGraph& scfg);

}  // namespace lucid
