#pragma once

#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {

// Converts `cfg` to Static Single Assignment (SSA) form.
void ConvertToStaticSingleAssignment(SyntaxContext& ctx, ControlFlowGraph& cfg);

}  // namespace lucid
