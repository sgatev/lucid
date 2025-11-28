#include "lucid/cfg_order.h"

#include <stack>
#include <vector>

#include "lucid/cfg.h"

namespace lucid {

using BlockRef = ControlFlowGraph::BlockRef;

std::vector<int> ComputeReversePostOrder(const ControlFlowGraph& cfg) {
  std::vector<int> post_order(cfg.blocks().Size(),
                              static_cast<int>(cfg.blocks().Size()));

  std::stack<BlockRef> pending;
  std::vector<int> visited(cfg.blocks().Size(), 0);

  int priority = static_cast<int>(cfg.blocks().Size() - 1);

  pending.push(cfg.first);
  visited[cfg.first.id()] = 1;

  while (!pending.empty()) {
    auto block = pending.top();
    pending.pop();

    if (visited[block.id()] == 2) {
      post_order[block.id()] = priority--;
    } else {
      pending.push(block);
      visited[block.id()] = 2;

      for (auto next_block : cfg.get(block).next) {
        if (visited[next_block.id()] == 0) {
          pending.push(next_block);
          visited[next_block.id()] = 1;
        }
      }
    }
  }

  return post_order;
}

}  // namespace lucid
