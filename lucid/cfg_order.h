#pragma once

#include <utility>
#include <vector>

#include "lucid/cfg.h"

namespace lucid {

// Function object for performing block order comparisons.
struct CompareBlockOrder {
  explicit CompareBlockOrder(std::vector<int> block_order)
      : block_order_(std::move(block_order)) {}

  bool operator()(const ControlFlowGraph::BlockRef& lhs,
                  const ControlFlowGraph::BlockRef& rhs) const {
    return block_order_[lhs.id()] < block_order_[rhs.id()];
  }

  std::vector<int> block_order_;
};

// Returns a vector whose elements correspond to reverse postorder traversal
// indices of blocks of `cfg`.
std::vector<int> ComputeReversePostOrder(const ControlFlowGraph& cfg);

}  // namespace lucid
