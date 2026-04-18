#pragma once

#include <algorithm>
#include <cassert>
#include <optional>
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
  const auto compare = CompareReversePostOrder(graph);

  std::vector<typename GraphT::vertex_type> vertices = Vertices(graph);
  std::sort(vertices.begin(), vertices.end(), compare);

  auto source_vertex = SourceVertex(graph);
  std::vector<std::optional<typename GraphT::vertex_type>> idoms(
      VertexCount(graph));
  idoms[VertexId(graph, source_vertex)] = source_vertex;

  bool changed = true;
  while (changed) {
    changed = false;

    for (auto vertex : vertices) {
      if (vertex == source_vertex) continue;

      std::optional<typename GraphT::vertex_type> new_idom;
      for (auto prev_vertex : PrevVertices(graph, vertex)) {
        if (!idoms[VertexId(graph, prev_vertex)].has_value()) continue;

        if (!new_idom.has_value()) {
          new_idom = prev_vertex;
        } else {
          while (new_idom != prev_vertex) {
            assert(new_idom.has_value());
            while (compare(prev_vertex, *new_idom)) {
              assert(new_idom.has_value());
              new_idom = idoms[VertexId(graph, *new_idom)];
            }
            assert(new_idom.has_value());
            while (compare(*new_idom, prev_vertex)) {
              assert(idoms[VertexId(graph, prev_vertex)].has_value());
              prev_vertex = *idoms[VertexId(graph, prev_vertex)];
            }
          }
        }
      }

      if (idoms[VertexId(graph, vertex)] != new_idom) {
        idoms[VertexId(graph, vertex)] = new_idom;
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
HashMap<typename GraphT::vertex_type, HashSet<typename GraphT::vertex_type>>
ComputeDominanceFrontiers(
    const GraphT& graph,
    const std::vector<std::optional<typename GraphT::vertex_type>>& idoms) {
  HashMap<typename GraphT::vertex_type, HashSet<typename GraphT::vertex_type>>
      dom_fronts;
  for (const auto& front_vertex : Vertices(graph)) {
    auto prev_vertices = PrevVertices(graph, front_vertex);
    if (prev_vertices.size() < 2) continue;

    for (auto prev_vertex : prev_vertices) {
      while (prev_vertex != idoms[VertexId(graph, front_vertex)]) {
        dom_fronts.Insert(prev_vertex, {});
        dom_fronts.Get(prev_vertex)->Insert(front_vertex);

        if (!idoms[VertexId(graph, prev_vertex)].has_value()) break;

        prev_vertex = *idoms[VertexId(graph, prev_vertex)];
      }
    }
  }
  return dom_fronts;
}

// Returns the dominator tree induced by `graph`.
template <Graph GraphT>
HashMap<typename GraphT::vertex_type, HashSet<typename GraphT::vertex_type>>
BuildDominatorTree(
    const GraphT& graph,
    const std::vector<std::optional<typename GraphT::vertex_type>>& idoms) {
  using vertex_type = typename GraphT::vertex_type;

  HashMap<vertex_type, HashSet<vertex_type>> dom_tree;
  for (vertex_type to : Vertices(graph)) {
    if (!idoms[VertexId(graph, to)].has_value()) continue;
    vertex_type from = *idoms[VertexId(graph, to)];
    dom_tree.Insert(from, {});
    dom_tree.Get(from)->Insert(to);
  }
  return dom_tree;
}

}  // namespace lucid
