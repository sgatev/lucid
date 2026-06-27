#pragma once

#include "lucid/syntax/cfg.h"
#include "lucid/syntax/context.h"

namespace lucid {

// Converts `scfg` to Static Single Assignment (SSA) form.
void ConvertToStaticSingleAssignment(SyntaxContext& syn_ctx,
                                     SyntaxControlFlowGraph& syn_cfg);

}  // namespace lucid
