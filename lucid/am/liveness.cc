#include "lucid/am/liveness.h"

#include <cassert>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/reg_set.h"

namespace lucid {
namespace {

using State = AbstractMachineLivenessAnalysis::State;
using Block = AbstractMachineControlFlowGraph::Block;

// Adds the registers that the phi functions of the blocks after `block` take
// from it. A phi reads its argument where control leaves the block that
// argument came from, so those registers are live where `block` exits.
void AddPhiSources(const AbstractMachineControlFlowGraph& am_cfg,
                   const Block& block, RegBitSet& regs) {
  for (const auto& succ_ref : block.succs) {
    const auto& succ_block = am_cfg.GetBlock(succ_ref);
    for (const auto& phi : succ_block.phis) {
      // An argument is paired with the predecessor it comes from, which is
      // this block at whichever position it holds among them.
      for (int i = 0; i < succ_block.preds.size(); ++i) {
        if (succ_block.preds[i] == block.ref) {
          regs.Insert(phi.srcs[i]);
          break;
        }
      }
    }
  }
}

}  // namespace

void AbstractMachineLivenessAnalysis::Transfer(State& state,
                                               const Instruction& inst) {
  TransferLive(state.live_in, inst);
}

AbstractMachineLivenessAnalysis::AbstractMachineLivenessAnalysis(
    const AbstractMachineControlFlowGraph& am_cfg)
    : am_cfg_(am_cfg) {}

State AbstractMachineLivenessAnalysis::Transfer(
    std::optional<State>&& prior_state,
    const AbstractMachineControlFlowGraph::BlockRef& block_ref) {
  State state;

  // What reaches the block is what is live where it exits, and the walk
  // backwards over it turns that into what is live where it enters.
  const auto& block = am_cfg_.GetBlock(block_ref);
  if (prior_state.has_value()) {
    state.live_out = std::move(prior_state->live_in);
    AddPhiSources(am_cfg_, block, state.live_out);
    state.live_in = state.live_out;
  }
  // The branch reads what it decides on where the block ends, after
  // everything the block does, so that is live to the end. An instruction
  // can follow the one that works it out, as the copy a short circuit makes
  // of a value it has already decided on does.
  if (block.branch_cond.has_value()) state.live_in.Insert(*block.branch_cond);
  for (const auto& inst : block.instructions | std::views::reverse) {
    Transfer(state, inst);
  }
  for (const auto& phi : block.phis) state.live_in.Remove(phi.dst);

  return state;
}

void AbstractMachineLivenessAnalysis::Join(State& left, const State& right) {
  left.live_in.InsertAll(right.live_in);
}

const RegBitSet& LiveOut(const std::vector<std::optional<State>>& states,
                         const Block& block) {
  assert(states[block.ref.id()].has_value());
  return states[block.ref.id()]->live_out;
}

}  // namespace lucid
