#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "lucid/core/dataflow/worklist.h"
#include "lucid/graph_order.h"

namespace lucid {

// A bounded join-semilattice.
template <typename L>
concept BoundedJoinSemiLattice = requires(L l1, L l2) {
  { l1 == l2 } -> std::same_as<bool>;
};

// A dataflow analysis over a graph `G`.
//
// Requires:
// - `State` sub-type that models a bounded join-semilattice.
// - `MakeInitial` member that constructs an initial state.
// - `Transfer` member that transfers a state given a vertex.
// - `Join` member that joins two states.
template <typename A, typename G>
concept DataflowAnalysis = Graph<G> and requires(A a, A::State s1, A::State s2,
                                                 const G::vertex_type& v) {
  requires BoundedJoinSemiLattice<typename A::State>;
  { a.MakeInitial() } -> std::same_as<typename A::State>;
  { a.Transfer(s1, v) } -> std::same_as<typename A::State>;
  { a.Join(s1, s2) } -> std::same_as<typename A::State>;
};

// A finite domain of vertices in a graph.
template <Graph GraphT>
class VertexDomain {
 public:
  explicit VertexDomain(const GraphT& graph)
      : vertex_count_(VertexCount(graph)) {}

  std::size_t size() const { return vertex_count_; }

  std::size_t id(GraphT::vertex_type vertex) const { return vertex.id(); }

 private:
  std::size_t vertex_count_;
};

// Performs backward dataflow analysis over a graph.
//
// Returns a mapping from vertex IDs to dataflow analysis states that model the
// respective vertices. The returned vector will have the same size as the
// number of vertices in the graph, with indices corresponding to vertex IDs.
template <Graph GraphT, DataflowAnalysis<GraphT> AnalysisT>
std::vector<std::optional<typename AnalysisT::State>> RunBackwardDataflow(
    const GraphT& graph, AnalysisT& analysis) {
  using State = typename AnalysisT::State;

  std::vector<std::optional<State>> states(VertexCount(graph));
  auto has_state = [&](auto ref) { return states[ref.id()].has_value(); };
  auto to_state = [&](auto ref) { return *states[ref.id()]; };

  Worklist<typename GraphT::vertex_type, VertexDomain<GraphT>,
           CompareVertexOrder<GraphT>>
      worklist(VertexDomain(graph),
               CompareVertexOrder<GraphT>(ComputeReversePostOrder(graph)));
  worklist.push(SinkVertex(graph));
  while (!worklist.empty()) {
    typename GraphT::vertex_type vertex = worklist.pop();
    State prior_state = std::ranges::fold_left(
        NextVertices(graph, vertex) | std::views::filter(has_state) |
            std::views::transform(to_state),
        analysis.MakeInitial(), std::bind_front(&AnalysisT::Join, &analysis));
    State new_state = analysis.Transfer(std::move(prior_state), vertex);
    if (auto& state = states[vertex.id()]; new_state != state) {
      state = std::move(new_state);
      worklist.push_range(PrevVertices(graph, vertex));
    }
  }

  return states;
}

}  // namespace lucid
