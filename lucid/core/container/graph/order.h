#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include "lucid/core/container/graph/graph.h"

namespace lucid {
namespace internal {

// Appends to `order` the vertices a depth-first walk of `g` from `from`
// finishes, in the order it finishes them. The walk skips the vertices
// `visited` marks, and marks the ones it comes to.
template <Graph GraphT>
void AppendPostOrder(const GraphT& g, typename GraphT::vertex_type from,
                     std::vector<bool>& visited,
                     std::vector<typename GraphT::vertex_type>& order) {
  // The vertices the walk has come to and not yet finished. A vector rather
  // than a `std::stack`, whose deque takes a large block to hold the first
  // vertex, and a walk is begun for every function there is.
  std::vector<typename GraphT::vertex_type> pending = {from};
  visited[VertexId(g, from)] = true;

  while (!pending.empty()) {
    const auto v = pending.back();

    // The walk goes on to the first vertex after `v` it has not come to, and
    // finishes `v` once there are none left.
    bool descended = false;
    for (auto n : NextVertices(g, v)) {
      if (visited[VertexId(g, n)]) continue;

      visited[VertexId(g, n)] = true;
      pending.push_back(n);
      descended = true;
      break;
    }
    if (descended) continue;

    pending.pop_back();
    order.push_back(v);
  }
}

// Returns every vertex of `g`: those reached from its source vertex in the
// order `arrange` puts the post-order of a walk from there in, then the rest.
template <Graph GraphT, typename ArrangeT>
std::vector<typename GraphT::vertex_type> ReachedThenRest(const GraphT& g,
                                                          ArrangeT arrange) {
  const std::size_t vertex_count = VertexCount(g);
  std::vector<bool> visited(vertex_count, false);
  std::vector<typename GraphT::vertex_type> order;
  order.reserve(vertex_count);

  AppendPostOrder(g, SourceVertex(g), visited, order);
  arrange(order);

  if (order.size() < vertex_count) {
    for (auto v : Vertices(g)) {
      if (!visited[VertexId(g, v)]) order.push_back(v);
    }
  }
  return order;
}

// Returns a vector indexed by vertex ID whose elements are the places the
// vertices of `g` take in `order`, which holds each of them once.
template <Graph GraphT>
std::vector<int> PlacesIn(
    const GraphT& g, const std::vector<typename GraphT::vertex_type>& order) {
  std::vector<int> places(VertexCount(g));
  for (std::size_t i = 0; i < order.size(); ++i) {
    places[VertexId(g, order[i])] = static_cast<int>(i);
  }
  return places;
}

}  // namespace internal

// Returns every vertex of `g`: those reached from its source vertex in
// post-order, then the rest.
template <Graph GraphT>
std::vector<typename GraphT::vertex_type> PostOrder(const GraphT& g) {
  return internal::ReachedThenRest(g, [](auto&) {});
}

// Returns every vertex of `g`: those reached from its source vertex in
// reverse post-order, then the rest.
template <Graph GraphT>
std::vector<typename GraphT::vertex_type> ReversePostOrder(const GraphT& g) {
  return internal::ReachedThenRest(
      g, [](auto& order) { std::reverse(order.begin(), order.end()); });
}

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
  return CompareVertexOrder<GraphT>(g, internal::PlacesIn(g, PostOrder(g)));
}

// Returns a function object for performing reverse post-order vertex
// comparisons on the given graph.
template <Graph GraphT>
CompareVertexOrder<GraphT> CompareReversePostOrder(const GraphT& g) {
  return CompareVertexOrder<GraphT>(g,
                                    internal::PlacesIn(g, ReversePostOrder(g)));
}

}  // namespace lucid
