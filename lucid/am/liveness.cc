#include "lucid/am/liveness.h"

#include <cassert>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {
namespace {

using State = AbstractMachineLivenessAnalysis::State;
using Block = AbstractMachineControlFlowGraph::Block;

// Adds the registers that the phi functions of the blocks after `block` take
// from it. A phi reads its argument where control leaves the block that
// argument came from, so those registers are live where `block` exits.
void AddPhiSources(const AbstractMachineControlFlowGraph& am_cfg,
                   const Block& block, HashSet<Reg>& regs) {
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
  if (auto reg = GetTargetRegister(inst); reg.has_value()) {
    state.live_in.Remove(*reg);
  }
  ForEachSourceRegister(inst, [&](Reg reg) { state.live_in.Insert(reg); });
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
    state.live_in = std::move(prior_state->live_in);
    AddPhiSources(am_cfg_, block, state.live_in);
  }
  for (const auto& inst : block.instructions | std::views::reverse) {
    Transfer(state, inst);
  }
  for (const auto& phi : block.phis) state.live_in.Remove(phi.dst);

  return state;
}

void AbstractMachineLivenessAnalysis::Join(State& left, const State& right) {
  for (Reg reg : right.live_in) left.live_in.Insert(reg);
}

HashSet<Reg> LiveOut(const AbstractMachineControlFlowGraph& am_cfg,
                     const std::vector<std::optional<State>>& states,
                     const Block& block) {
  HashSet<Reg> live_out;
  for (const auto& succ_ref : block.succs) {
    const auto& succ_state = states[succ_ref.id()];
    if (!succ_state.has_value()) continue;

    for (Reg reg : succ_state->live_in) live_out.Insert(reg);
  }
  AddPhiSources(am_cfg, block, live_out);
  return live_out;
}

}  // namespace lucid
