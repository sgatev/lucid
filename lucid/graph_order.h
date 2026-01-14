#pragma once

#include <cstddef>
#include <stack>
#include <utility>
#include <vector>

#include "lucid/graph.h"

namespace lucid {

// Function object for performing vertex comparisons.
template <Graph GraphT>
struct CompareVertexOrder {
  explicit CompareVertexOrder(std::vector<int> vertex_order)
      : vertex_order_(std::move(vertex_order)) {}

  bool operator()(const GraphT::vertex_type& lhs,
                  const GraphT::vertex_type& rhs) const {
    return vertex_order_[lhs.id()] < vertex_order_[rhs.id()];
  }

  std::vector<int> vertex_order_;
};

// Returns a vector whose elements correspond to reverse postorder traversal
// indices of blocks of `cfg`.
template <Graph GraphT>
inline std::vector<int> ComputeReversePostOrder(const GraphT& graph) {
  const std::size_t vertex_count = VertexCount(graph);
  std::vector<int> post_order(vertex_count, static_cast<int>(vertex_count));

  std::stack<typename GraphT::vertex_type> pending;
  std::vector<int> visited(vertex_count, 0);

  int priority = static_cast<int>(vertex_count - 1);

  pending.push(SourceVertex(graph));
  visited[SourceVertex(graph).id()] = 1;

  while (!pending.empty()) {
    auto block = pending.top();
    pending.pop();

    if (visited[block.id()] == 2) {
      post_order[block.id()] = priority--;
    } else {
      pending.push(block);
      visited[block.id()] = 2;

      for (auto next_block : NextVertices(graph, block)) {
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
