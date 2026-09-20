#include "lucid/am/liveness.h"

#include <cassert>
#include <optional>
#include <ranges>
#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {

using State = AbstractMachineLivenessAnalysis::State;

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
  const auto& block = am_cfg_.GetBlock(block_ref);
  State state;

  if (prior_state.has_value()) {
    state.live_out = std::move(prior_state->live_in);

    for (const auto& succ_ref : block.succs) {
      const auto& succ_block = am_cfg_.GetBlock(succ_ref);
      for (const auto& phi : succ_block.phis) {
        for (int i = 0; i < succ_block.preds.size(); ++i) {
          if (succ_block.preds[i] == block.ref) {
            state.live_out.Insert(phi.srcs[i]);
            break;
          }
        }
      }
    }

    state.live_in = state.live_out;
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

}  // namespace lucid
