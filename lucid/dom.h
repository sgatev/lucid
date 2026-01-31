#pragma once

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lucid/cfg.h"
#include "lucid/graph.h"
#include "lucid/graph_order.h"

namespace lucid {

// Returns a vector indexed by ids of vertices in `graph` whose values are the
// respective immediate dominators of the vertices, i.e. their parent nodes in
// the dominator tree induced by `graph`.
template <Graph GraphT>
inline std::vector<std::optional<typename GraphT::vertex_type>>
ComputeImmediateDominators(const GraphT& graph) {
  const CompareVertexOrder<GraphT> compare(ComputeReversePostOrder(graph));

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

// Returns a map from basic blocks in `cfg` to their respective dominance
// frontiers. Only blocks with non-empty dominance frontiers are represented in
// the map.
//
// Requires:
// - `idoms` must be the immediate dominators computed from `cfg`.
std::unordered_map<ControlFlowGraph::BlockRef,
                   std::unordered_set<ControlFlowGraph::BlockRef>>
ComputeDominanceFrontiers(
    const ControlFlowGraph& cfg,
    const std::vector<std::optional<ControlFlowGraph::BlockRef>>& idoms);

}  // namespace lucid
