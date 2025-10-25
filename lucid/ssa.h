#pragma once

#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {

// Converts `cfg` to Static Single Assignment (SSA) form.
void ConvertToStaticSingleAssignment(SyntaxContext& ctx, ControlFlowGraph& cfg);

// Removes Phi functions associated with Static Single Assigment (SSA) from
// `cfg`.
void DestroyStaticSingleAssignment(SyntaxContext& ctx, ControlFlowGraph& cfg);

}  // namespace lucid
