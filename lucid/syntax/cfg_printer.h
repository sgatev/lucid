#pragma once

#include <ostream>

#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {

// Prints a text representation of the syntax control flow graph `scfg` to
// `out`.
//
// Requires:
// - `scfg` must be constructed in `ctx`.
void Print(const SyntaxContext& ctx, const SyntaxControlFlowGraph& scfg,
           std::ostream& out);

}  // namespace lucid
