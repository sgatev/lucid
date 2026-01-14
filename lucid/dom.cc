#include "lucid/dom.h"

#include <algorithm>
#include <vector>

#include "lucid/cfg.h"
#include "lucid/graph_order.h"

namespace lucid {

using BlockRef = ControlFlowGraph::BlockRef;

static constexpr auto kNullBlockRef = ControlFlowGraph::kNullBlockRef;

std::vector<BlockRef> ComputeImmediateDominators(const ControlFlowGraph& cfg) {
  const CompareVertexOrder<ControlFlowGraph> compare(
      ComputeReversePostOrder(cfg));

  std::vector<BlockRef> blocks;
  for (const auto& block : cfg.blocks()) blocks.push_back(block.ref.id());
  std::sort(blocks.begin(), blocks.end(), compare);

  std::vector<BlockRef> idoms(cfg.blocks().Size(), kNullBlockRef);
  idoms[cfg.first.id()] = cfg.first;

  bool changed = true;
  while (changed) {
    changed = false;

    for (auto block : blocks) {
      if (block == cfg.first) continue;

      auto new_idom = kNullBlockRef;
      for (auto pred : cfg.get(block).preds) {
        if (idoms[pred.id()] == kNullBlockRef) continue;

        if (new_idom == kNullBlockRef) {
          new_idom = pred;
        } else {
          while (new_idom != pred) {
            while (compare(pred, new_idom)) {
              new_idom = idoms[new_idom.id()];
            }
            while (compare(new_idom, pred)) {
              pred = idoms[pred.id()];
            }
          }
        }
      }

      if (idoms[block.id()] != new_idom) {
        idoms[block.id()] = new_idom;
        changed = true;
      }
    }
  }

  return idoms;
}

std::unordered_map<BlockRef, std::unordered_set<BlockRef>>
ComputeDominanceFrontiers(const ControlFlowGraph& cfg,
                          const std::vector<BlockRef>& idoms) {
  std::unordered_map<BlockRef, std::unordered_set<BlockRef>> dom_fronts;
  for (const auto& front_block : cfg.blocks()) {
    if (front_block.preds.size() < 2) continue;

    for (BlockRef pred : front_block.preds) {
      for (; pred != kNullBlockRef && pred != idoms[front_block.ref.id()];
           pred = idoms[pred.id()]) {
        dom_fronts[pred].insert(front_block.ref);
      }
    }
  }
  return dom_fronts;
}

}  // namespace lucid
