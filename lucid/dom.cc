#include "lucid/dom.h"

#include <vector>

#include "lucid/cfg.h"

namespace lucid {

using BlockRef = ControlFlowGraph::BlockRef;

static constexpr auto kNullBlockRef = ControlFlowGraph::kNullBlockRef;

std::unordered_map<BlockRef, std::unordered_set<BlockRef>>
ComputeDominanceFrontiers(const ControlFlowGraph& cfg,
                          const std::vector<std::optional<BlockRef>>& idoms) {
  std::unordered_map<BlockRef, std::unordered_set<BlockRef>> dom_fronts;
  for (const auto& front_block : cfg.blocks()) {
    if (front_block.preds.size() < 2) continue;

    for (BlockRef pred : front_block.preds) {
      while (pred != kNullBlockRef && pred != idoms[front_block.ref.id()]) {
        dom_fronts[pred].insert(front_block.ref);

        if (!idoms[pred.id()].has_value()) break;

        pred = *idoms[pred.id()];
      }
    }
  }
  return dom_fronts;
}

}  // namespace lucid
