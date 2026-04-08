#pragma once

#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {

// Converts `scfg` to Static Single Assignment (SSA) form.
void ConvertToStaticSingleAssignment(SyntaxContext& ctx,
                                     SyntaxControlFlowGraph& scfg);

}  // namespace lucid
