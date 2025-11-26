#include "lucid/dom.h"

#include <vector>

#include "lucid/cfg.h"
#include "lucid/cfg_order.h"

namespace lucid {

using BlockRef = ControlFlowGraph::BlockRef;

static constexpr auto kNullBlockRef = ControlFlowGraph::kNullBlockRef;

std::vector<BlockRef> ComputeImmediateDominators(const ControlFlowGraph& cfg) {
  const std::vector<BlockRef> reverse_post_order = ComputeReversePostOrder(cfg);

  std::vector<int> block_order(cfg.blocks().Size(), -1);
  for (int i = 0; i < reverse_post_order.size(); ++i) {
    block_order[reverse_post_order[i].id()] = i;
  }

  std::vector<BlockRef> idoms(cfg.blocks().Size(), kNullBlockRef);
  idoms[cfg.first.id()] = cfg.first;

  bool changed = true;
  while (changed) {
    changed = false;

    for (auto block : reverse_post_order) {
      if (block == cfg.first) continue;

      auto new_idom = kNullBlockRef;
      for (auto pred : cfg.get(block).preds) {
        if (idoms[pred.id()] == kNullBlockRef) continue;

        if (new_idom == kNullBlockRef) {
          new_idom = pred;
        } else {
          while (new_idom != pred) {
            while (block_order[new_idom.id()] > block_order[pred.id()]) {
              new_idom = idoms[new_idom.id()];
            }
            while (block_order[pred.id()] > block_order[new_idom.id()]) {
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
