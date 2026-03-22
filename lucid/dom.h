#pragma once

#include <algorithm>
#include <cassert>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lucid/core/container/graph/graph.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// Returns a vector indexed by ids of vertices in `graph` whose values are the
// respective immediate dominators of the vertices, i.e. their parent nodes in
// the dominator tree induced by `graph`.
template <Graph GraphT>
std::vector<std::optional<typename GraphT::vertex_type>>
ComputeImmediateDominators(const GraphT& graph) {
  const CompareVertexOrder<GraphT> compare(graph,
                                           ComputeReversePostOrder(graph));

  std::vector<typename GraphT::vertex_type> vertices = Vertices(graph);
  std::sort(vertices.begin(), vertices.end(), compare);

  auto source_vertex = SourceVertex(graph);
  std::vector<std::optional<typename GraphT::vertex_type>> idoms(
      VertexCount(graph));
  idoms[source_vertex.id()] = source_vertex;

  bool changed = true;
  while (changed) {
    changed = false;

    for (auto vertex : vertices) {
      if (vertex == source_vertex) continue;

      std::optional<typename GraphT::vertex_type> new_idom;
      for (auto prev_vertex : PrevVertices(graph, vertex)) {
        if (!idoms[prev_vertex.id()].has_value()) continue;

        if (!new_idom.has_value()) {
          new_idom = prev_vertex;
        } else {
          while (new_idom != prev_vertex) {
            assert(new_idom.has_value());
            while (compare(prev_vertex, *new_idom)) {
              assert(new_idom.has_value());
              new_idom = idoms[new_idom->id()];
            }
            assert(new_idom.has_value());
            while (compare(*new_idom, prev_vertex)) {
              assert(idoms[prev_vertex.id()].has_value());
              prev_vertex = *idoms[prev_vertex.id()];
            }
          }
        }
      }

      if (idoms[vertex.id()] != new_idom) {
        idoms[vertex.id()] = new_idom;
        changed = true;
      }
    }
  }

  return idoms;
}

// Returns a map from vertices in `graph` to their respective dominance
// frontiers. Only vertices with non-empty dominance frontiers are represented
// in the map.
//
// Requires:
// - `idoms` must be the immediate dominators computed from `graph`.
template <Graph GraphT>
std::unordered_map<typename GraphT::vertex_type,
                   std::unordered_set<typename GraphT::vertex_type>>
ComputeDominanceFrontiers(
    const GraphT& graph,
    const std::vector<std::optional<typename GraphT::vertex_type>>& idoms) {
  std::unordered_map<typename GraphT::vertex_type,
                     std::unordered_set<typename GraphT::vertex_type>>
      dom_fronts;
  for (const auto& front_vertex : Vertices(graph)) {
    auto prev_vertices = PrevVertices(graph, front_vertex);
    if (prev_vertices.size() < 2) continue;

    for (auto prev_vertex : prev_vertices) {
      while (prev_vertex != idoms[front_vertex.id()]) {
        dom_fronts[prev_vertex].insert(front_vertex);

        if (!idoms[prev_vertex.id()].has_value()) break;

        prev_vertex = *idoms[prev_vertex.id()];
      }
    }
  }
  return dom_fronts;
}

// Returns the dominator tree induced by `graph`.
template <Graph GraphT>
HashMap<typename GraphT::vertex_type, HashSet<typename GraphT::vertex_type>>
BuildDominatorTree(const GraphT& graph) {
  using vertex_type = typename GraphT::vertex_type;

  HashMap<vertex_type, HashSet<vertex_type>> dom_tree;
  std::vector<std::optional<vertex_type>> idoms =
      ComputeImmediateDominators(graph);
  for (vertex_type to : Vertices(graph)) {
    if (!idoms[to.id()].has_value()) continue;
    vertex_type from = *idoms[to.id()];
    dom_tree.Insert(from, {});
    dom_tree.Find(from)->Insert(to);
  }
  return dom_tree;
}

}  // namespace lucid
