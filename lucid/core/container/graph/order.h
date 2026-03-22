#pragma once

#include <cstddef>
#include <stack>
#include <utility>
#include <vector>

#include "lucid/core/container/graph/graph.h"

namespace lucid {

// Function object for performing vertex comparisons.
template <Graph GraphT>
struct CompareVertexOrder {
  explicit CompareVertexOrder(const GraphT& graph,
                              std::vector<int> vertex_order)
      : graph_(graph), vertex_order_(std::move(vertex_order)) {}

  bool operator()(const GraphT::vertex_type& lhs,
                  const GraphT::vertex_type& rhs) const {
    return vertex_order_[VertexId(graph_, lhs)] <
           vertex_order_[VertexId(graph_, rhs)];
  }

  const GraphT& graph_;
  std::vector<int> vertex_order_;
};

// Returns a vector whose elements correspond to reverse postorder traversal
// indices of blocks of `cfg`.
template <Graph GraphT>
inline std::vector<int> ComputeReversePostOrder(const GraphT& g) {
  const std::size_t vertex_count = VertexCount(g);
  std::vector<int> post_order(vertex_count, static_cast<int>(vertex_count));

  std::stack<typename GraphT::vertex_type> pending;
  std::vector<int> visited(vertex_count, 0);

  int priority = static_cast<int>(vertex_count - 1);

  pending.push(SourceVertex(g));
  visited[VertexId(g, SourceVertex(g))] = 1;

  while (!pending.empty()) {
    auto v = pending.top();
    pending.pop();

    if (visited[VertexId(g, v)] == 2) {
      post_order[VertexId(g, v)] = priority--;
    } else {
      pending.push(v);
      visited[VertexId(g, v)] = 2;

      for (auto n : NextVertices(g, v)) {
        if (visited[VertexId(g, n)] == 0) {
          pending.push(n);
          visited[VertexId(g, n)] = 1;
        }
      }
    }
  }

  return post_order;
}

}  // namespace lucid
