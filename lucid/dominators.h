#pragma once

#include <vector>

#include "lucid/cfg.h"

namespace lucid {

// Returns a vector indexed by ids of basic blocks in `cfg` whose values are the
// respective immediate dominators of the blocks, i.e. their parent nodes in the
// dominator tree induced by `cfg`.
std::vector<ControlFlowGraph::BlockRef> ComputeImmediateDominators(
    const ControlFlowGraph& cfg);

}  // namespace lucid
