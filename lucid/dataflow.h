#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "lucid/cfg.h"
#include "lucid/cfg_order.h"
#include "lucid/worklist.h"

namespace lucid {

// A bounded join-semilattice.
template <typename L>
concept BoundedJoinSemiLattice = requires(L l1, L l2) {
  { l1 == l2 } -> std::same_as<bool>;
};

// A dataflow analysis.
//
// Requires:
// - `State` sub-type that models a bounded join-semilattice.
// - `MakeInitial` member that constructs an initial state.
// - `Transfer` member that transfers a state given a sequence.
// - `Join` member that joins two states.
template <typename A>
concept DataflowAnalysis = requires(A a, A::State s1, A::State s2,
                                    const ControlFlowGraph::Sequence& seq) {
  requires BoundedJoinSemiLattice<typename A::State>;
  { a.MakeInitial() } -> std::same_as<typename A::State>;
  { a.Transfer(s1, seq) } -> std::same_as<typename A::State>;
  { a.Join(s1, s2) } -> std::same_as<typename A::State>;
};

// Performs backward dataflow analysis and returns a mapping from basic block
// IDs to dataflow analysis states that model the respective basic blocks.
//
// The returned vector will have the same size as the number of CFG blocks, with
// indices corresponding to basic block IDs.
template <DataflowAnalysis AnalysisT>
std::vector<std::optional<typename AnalysisT::State>> RunBackwardDataflow(
    const ControlFlowGraph& cfg, AnalysisT& analysis) {
  using Block = ControlFlowGraph::Block;
  using BlockRef = ControlFlowGraph::BlockRef;
  using State = typename AnalysisT::State;

  std::vector<std::optional<State>> block_states(cfg.blocks().Size());
  auto block_to_state = [&](BlockRef ref) { return *block_states[ref.id()]; };

  Worklist<BlockRef, CompareBlockOrder> worklist(
      CompareBlockOrder(ComputeReversePostOrder(cfg)));
  worklist.push(cfg.last);

  while (!worklist.empty()) {
    const Block& block = cfg.get(worklist.pop());
    State prior_state = std::ranges::fold_left(
        block.next | std::views::transform(block_to_state),
        analysis.MakeInitial(), std::bind_front(&AnalysisT::Join, &analysis));
    State new_state = std::ranges::fold_left(
        block.sequences | std::views::reverse, std::move(prior_state),
        std::bind_front(&AnalysisT::Transfer, &analysis));
    auto& state = block_states[block.ref.id()];
    if (new_state != state) {
      state = std::move(new_state);
      worklist.push_range(block.preds);
    }
  }

  return block_states;
}

}  // namespace lucid
