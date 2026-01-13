#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
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

// A finite domain of blocks in a control-flow graph.
class BlockDomain {
 public:
  explicit BlockDomain(const ControlFlowGraph& cfg)
      : blocks_count_(cfg.blocks().Size()) {}

  std::size_t size() const { return blocks_count_; }

  std::size_t id(ControlFlowGraph::BlockRef ref) const { return ref.id(); }

 private:
  std::size_t blocks_count_;
};

// Performs backward dataflow analysis and returns a mapping from basic block
// IDs to dataflow analysis states that model the respective basic blocks.
//
// The returned vector will have the same size as the number of CFG blocks, with
// indices corresponding to basic block IDs.
template <DataflowAnalysis AnalysisT>
std::vector<std::optional<typename AnalysisT::State>> RunBackwardDataflow(
    const ControlFlowGraph& cfg, AnalysisT& analysis) {
  using State = typename AnalysisT::State;

  std::vector<std::optional<State>> block_states(cfg.blocks().Size());
  auto has_state = [&](auto ref) { return block_states[ref.id()].has_value(); };
  auto to_state = [&](auto ref) { return *block_states[ref.id()]; };

  Worklist<ControlFlowGraph::BlockRef, BlockDomain, CompareBlockOrder> worklist(
      BlockDomain(cfg), CompareBlockOrder(ComputeReversePostOrder(cfg)));
  worklist.push(cfg.last);
  while (!worklist.empty()) {
    const ControlFlowGraph::Block& block = cfg.get(worklist.pop());
    State prior_state = std::ranges::fold_left(
        block.next | std::views::filter(has_state) |
            std::views::transform(to_state),
        analysis.MakeInitial(), std::bind_front(&AnalysisT::Join, &analysis));
    State new_state = std::ranges::fold_left(
        block.sequences | std::views::reverse, std::move(prior_state),
        std::bind_front(&AnalysisT::Transfer, &analysis));
    if (auto& state = block_states[block.ref.id()]; new_state != state) {
      state = std::move(new_state);
      worklist.push_range(block.preds);
    }
  }

  return block_states;
}

}  // namespace lucid
