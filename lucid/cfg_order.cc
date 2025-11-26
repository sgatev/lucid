#include "lucid/cfg_order.h"

#include <algorithm>
#include <stack>
#include <vector>

#include "lucid/cfg.h"

namespace lucid {

using BlockRef = ControlFlowGraph::BlockRef;

std::vector<int> ComputeReversePreOrder(const ControlFlowGraph& cfg) {
  int priority = static_cast<int>(cfg.blocks().Size());

  std::vector<int> pre_order(cfg.blocks().Size(), priority--);

  std::stack<BlockRef> pending;
  std::vector<bool> visited(cfg.blocks().Size(), false);

  pending.push(cfg.first);
  while (!pending.empty()) {
    auto block = pending.top();
    pending.pop();

    if (visited[block.id()]) continue;

    pre_order[block.id()] = priority--;
    for (auto next_block : cfg.get(block).next) {
      if (!visited[next_block.id()]) pending.push(next_block);
    }
    visited[block.id()] = true;
  }

  return pre_order;
}

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

}  // namespace lucid
