#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <queue>
#include <ranges>
#include <unordered_set>
#include <utility>
#include <vector>

#include "lucid/cfg.h"
#include "lucid/cfg_order.h"

namespace lucid {

// A bounded join-semilattice.
template <typename L>
concept BoundedJoinSemiLattice = requires(L l1, L l2) {
  { l1 == l2 } -> std::same_as<bool>;
};

// A dataflow analysis.
//
// Require:
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

// Function object for performing block order comparisons.
struct CompareBlockOrder {
  explicit CompareBlockOrder(std::vector<int> block_order)
      : block_order_(std::move(block_order)) {}

  bool operator()(const ControlFlowGraph::BlockRef& lhs,
                  const ControlFlowGraph::BlockRef& rhs) const {
    return block_order_[lhs.id()] < block_order_[rhs.id()];
  }

  std::vector<int> block_order_;
};

// A worklist of elements of type `T` ordered using comparator of type `C`. An
// element can appear at most once in the worklist.
template <typename T, typename C>
class Worklist {
 public:
  explicit Worklist(C comp) : queue_(std::move(comp)) {}

  // Returns whether the worklist is empty.
  bool empty() const { return queue_.empty(); }

  // Removes and returns the top element in the worklist.
  T pop() {
    T top = queue_.top();
    queue_.pop();
    inserted_.erase(top);
    return top;
  }

  // Inserts and sorts the element in the worklist if it's not already present.
  void push(T t) {
    if (inserted_.insert(t).second) queue_.push(t);
  }

  // Inserts and sorts the elements from the range that are not already present
  // in the worklist.
  template <typename R>
  void push_range(R&& rg) {
    queue_.push_range(rg | std::views::filter([&](const T& t) {
                        return !inserted_.contains(t);
                      }));
    inserted_.insert_range(rg);
  }

 private:
  std::priority_queue<T, std::vector<T>, C> queue_;
  std::unordered_set<T> inserted_;
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
      CompareBlockOrder(ComputeReversePreOrder(cfg)));
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
