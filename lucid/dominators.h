#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lucid/cfg.h"

namespace lucid {

// Returns a vector indexed by ids of basic blocks in `cfg` whose values are the
// respective immediate dominators of the blocks, i.e. their parent nodes in the
// dominator tree induced by `cfg`.
std::vector<ControlFlowGraph::BlockRef> ComputeImmediateDominators(
    const ControlFlowGraph& cfg);

// Returns a map from basic blocks in `cfg` to their respective dominance
// frontiers. Only blocks with non-empty dominance frontiers are represented in
// the map.
//
// Requires:
// - `idoms` must be the immediate dominators computed from `cfg`.
std::unordered_map<ControlFlowGraph::BlockRef,
                   std::unordered_set<ControlFlowGraph::BlockRef>>
ComputeDominanceFrontiers(const ControlFlowGraph& cfg,
                          const std::vector<ControlFlowGraph::BlockRef>& idoms);

}  // namespace lucid
