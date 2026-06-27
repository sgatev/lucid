#pragma once

#include <ostream>

#include "lucid/syntax/cfg.h"
#include "lucid/syntax/context.h"

namespace lucid {

// Prints a text representation of the syntax control flow graph `syn_cfg` to
// `out`.
//
// Requires:
// - `syn_cfg` must be constructed in `syn_ctx`.
void Print(const SyntaxContext& syn_ctx, const SyntaxControlFlowGraph& syn_cfg,
           std::ostream& out);

}  // namespace lucid
