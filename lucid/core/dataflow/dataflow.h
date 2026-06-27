#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "lucid/core/container/graph/graph.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/dataflow/worklist.h"

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
// - `Transfer` member that transfers a prior state given a vertex.
// - `Join` member that joins two prior states as input for `Transfer`.
template <typename A, typename G>
concept DataflowAnalysis =
    Graph<G> and requires(A a, std::optional<typename A::State> os, A::State s1,
                          A::State s2, const G::vertex_type& v) {
      requires BoundedJoinSemiLattice<typename A::State>;
      { a.Transfer(std::move(os), v) } -> std::same_as<typename A::State>;
      { a.Join(std::move(s1), s2) } -> std::same_as<typename A::State>;
    };

// A finite domain of vertices in a graph.
template <Graph GraphT>
class VertexDomain {
 public:
  using element_type = GraphT::vertex_type;

  explicit VertexDomain(const GraphT& graph)
      : graph_(graph), vertex_count_(VertexCount(graph)) {}

  std::size_t size() const { return vertex_count_; }

  std::size_t id(GraphT::vertex_type vertex) const {
    return VertexId(graph_, vertex);
  }

 private:
  const GraphT& graph_;
  std::size_t vertex_count_;
};

// A scheme for dataflow analysis.
//
// Requires:
// - `Graph` sub-type that models a single-source, single-sink graph.
// - `Compare` member that returns a function object for performing vertex
//   comparisons.
// - `Domain` member that returns a finite domain of vertices in the graph.
// - `Initial` member that returns the initial vertex of the graph.
// - `Prior` member that returns the vertices that come before a given vertex in
//   the graph.
// - `Subsequent` member that returns the vertices that come after a given
//   vertex in the graph.
template <typename S>
concept DataflowScheme = requires(S s, typename S::Graph::vertex_type v) {
  requires Graph<typename S::Graph>;
  { s.Compare() } -> std::same_as<CompareVertexOrder<typename S::Graph>>;
  { s.Domain() } -> std::same_as<VertexDomain<typename S::Graph>>;
  { s.Compare() } -> std::same_as<CompareVertexOrder<typename S::Graph>>;
  { s.Initial() } -> std::same_as<typename S::Graph::vertex_type>;
  { s.Prior(v) } -> std::same_as<std::vector<typename S::Graph::vertex_type>>;
  {
    s.Subsequent(v)
  } -> std::same_as<std::vector<typename S::Graph::vertex_type>>;
};

// A scheme for forward dataflow analysis.
template <Graph GraphT>
struct Forward {
  using Graph = GraphT;

  Forward(const GraphT& g) : g_(g) {}

  VertexDomain<GraphT> Domain() const { return VertexDomain(g_); }

  CompareVertexOrder<GraphT> Compare() const {
    return CompareReversePostOrder(g_);
  }

  typename GraphT::vertex_type Initial() const { return SourceVertex(g_); }

  std::vector<typename GraphT::vertex_type> Prior(
      typename GraphT::vertex_type v) const {
    return PrevVertices(g_, v);
  }

  std::vector<typename GraphT::vertex_type> Subsequent(
      typename GraphT::vertex_type v) const {
    return NextVertices(g_, v);
  }

 private:
  const GraphT& g_;
};

// A scheme for backward dataflow analysis.
template <Graph GraphT>
struct Backward {
  using Graph = GraphT;

  Backward(const GraphT& g) : g_(g) {}

  VertexDomain<GraphT> Domain() const { return VertexDomain(g_); }

  CompareVertexOrder<GraphT> Compare() const { return ComparePostOrder(g_); }

  typename GraphT::vertex_type Initial() const { return SinkVertex(g_); }

  std::vector<typename GraphT::vertex_type> Prior(
      typename GraphT::vertex_type v) const {
    return NextVertices(g_, v);
  }

  std::vector<typename GraphT::vertex_type> Subsequent(
      typename GraphT::vertex_type v) const {
    return PrevVertices(g_, v);
  }

 private:
  const GraphT& g_;
};

// Left-folds the elements of given range. Returns nullopt if the range is
// empty.
template <std::ranges::input_range R, typename F>
std::optional<std::ranges::range_value_t<R>> fold_left_first(R&& r, F&& f) {
  if (std::ranges::empty(r)) return std::nullopt;
  auto begin = std::ranges::begin(r);
  return std::ranges::fold_left(
      std::ranges::subrange(std::ranges::next(begin), std::ranges::end(r)),
      *begin, f);
}

// Performs dataflow analysis over a graph according to the given scheme.
//
// Returns a mapping from vertex IDs to dataflow analysis states that model the
// respective vertices. The returned vector will have the same size as the
// number of vertices in the graph, with indices corresponding to vertex IDs.
template <DataflowScheme SchemeT,
          DataflowAnalysis<typename SchemeT::Graph> AnalysisT>
std::vector<std::optional<typename AnalysisT::State>> RunDataflow(
    const SchemeT& scheme, AnalysisT& analysis) {
  using GraphT = typename SchemeT::Graph;
  using State = typename AnalysisT::State;

  auto domain = scheme.Domain();

  std::vector<std::optional<State>> states(domain.size());
  auto has_state = [&](auto v) { return states[domain.id(v)].has_value(); };
  auto to_state = [&](auto v) { return *states[domain.id(v)]; };

  Worklist vertices_to_process(domain, scheme.Compare());
  vertices_to_process.push(scheme.Initial());
  while (!vertices_to_process.empty()) {
    typename GraphT::vertex_type vertex = vertices_to_process.pop();

    std::optional<State> prior_state =
        fold_left_first(scheme.Prior(vertex) | std::views::filter(has_state) |
                            std::views::transform(to_state),
                        std::bind_front(&AnalysisT::Join, &analysis));

    State new_state = analysis.Transfer(std::move(prior_state), vertex);
    if (auto& state = states[domain.id(vertex)]; new_state != state) {
      state = std::move(new_state);
      vertices_to_process.push_range(scheme.Subsequent(vertex));
    }
  }

  return states;
}

}  // namespace lucid
