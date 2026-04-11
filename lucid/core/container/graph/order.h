#pragma once

#include <cstddef>
#include <stack>
#include <utility>
#include <vector>

#include "lucid/core/container/graph/graph.h"

namespace lucid {
namespace internal {

// Returns a vector whose elements correspond to post-order traversal
// indices of blocks of `cfg`.
template <Graph GraphT>
std::vector<int> ComputePostOrder(const GraphT& g) {
  const std::size_t vertex_count = VertexCount(g);
  std::vector<int> post_order(vertex_count, static_cast<int>(vertex_count));

  std::stack<typename GraphT::vertex_type> pending;
  std::vector<int> visited(vertex_count, 0);

  int priority = 0;

  pending.push(SourceVertex(g));
  visited[VertexId(g, SourceVertex(g))] = 1;

  while (!pending.empty()) {
    auto v = pending.top();
    pending.pop();

    bool inserted = false;
    for (auto n : NextVertices(g, v)) {
      if (visited[VertexId(g, n)] == 1) continue;

      pending.push(v);
      visited[VertexId(g, v)] = 1;

      pending.push(n);
      visited[VertexId(g, n)] = 1;

      inserted = true;
      break;
    }
    if (!inserted) post_order[VertexId(g, v)] = priority++;
  }

  return post_order;
}

// Returns a vector whose elements correspond to reverse post-order traversal
// indices of blocks of `cfg`.
template <Graph GraphT>
std::vector<int> ComputeReversePostOrder(const GraphT& g) {
  const std::size_t vertex_count = VertexCount(g);
  std::vector<int> reverse_post_order(vertex_count,
                                      static_cast<int>(vertex_count));

  std::stack<typename GraphT::vertex_type> pending;
  std::vector<int> visited(vertex_count, 0);

  int priority = static_cast<int>(vertex_count - 1);

  pending.push(SourceVertex(g));
  visited[VertexId(g, SourceVertex(g))] = 1;

  while (!pending.empty()) {
    auto v = pending.top();
    pending.pop();

    bool inserted = false;
    for (auto n : NextVertices(g, v)) {
      if (visited[VertexId(g, n)] == 1) continue;

      pending.push(v);
      visited[VertexId(g, v)] = 1;

      pending.push(n);
      visited[VertexId(g, n)] = 1;

      inserted = true;
      break;
    }
    if (!inserted) reverse_post_order[VertexId(g, v)] = priority--;
  }

  return reverse_post_order;
}

}  // namespace internal

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

// Returns a unction object for performing post-order vertex comparisons on the
// given graph.
template <Graph GraphT>
CompareVertexOrder<GraphT> ComparePostOrder(const GraphT& g) {
  return CompareVertexOrder<GraphT>(g, internal::ComputePostOrder(g));
}

// Returns a function object for performing reverse post-order vertex
// comparisons on the given graph.
template <Graph GraphT>
CompareVertexOrder<GraphT> CompareReversePostOrder(const GraphT& g) {
  return CompareVertexOrder<GraphT>(g, internal::ComputeReversePostOrder(g));
}

}  // namespace lucid
