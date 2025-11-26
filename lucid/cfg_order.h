#pragma once

#include <vector>

#include "lucid/cfg.h"

namespace lucid {

// Returns a vector whose elements correspond to reverse preorder traversal
// indices of blocks of `cfg`.
std::vector<int> ComputeReversePreOrder(const ControlFlowGraph& cfg);

std::vector<ControlFlowGraph::BlockRef> ComputeReversePostOrder(
    const ControlFlowGraph& cfg);

}  // namespace lucid
