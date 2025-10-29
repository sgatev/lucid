#include "lucid/dom.h"

#include <algorithm>
#include <stack>
#include <vector>

#include "lucid/cfg.h"

namespace lucid {
namespace {

using BlockRef = ControlFlowGraph::BlockRef;

static constexpr auto kNullBlockRef = ControlFlowGraph::kNullBlockRef;

std::vector<BlockRef> ComputeReversePostOrder(const ControlFlowGraph& cfg) {
  std::vector<BlockRef> reverse_post_order;
  reverse_post_order.reserve(cfg.blocks().Size());

  std::stack<BlockRef> pending;
  std::vector<bool> visited(cfg.blocks().Size(), false);

  pending.push(cfg.first);
  while (!pending.empty()) {
    auto block = pending.top();
    pending.pop();

    if (visited[block.id()]) {
      reverse_post_order.push_back(block);
    } else {
      pending.push(block);
      for (auto next_block : cfg.get(block).next) {
        if (!visited[next_block.id()]) pending.push(next_block);
      }
      visited[block.id()] = true;
    }
  }

  std::reverse(reverse_post_order.begin(), reverse_post_order.end());

  return reverse_post_order;
}

}  // namespace

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
