#include "lucid/am/liveness.h"

#include <cassert>
#include <optional>
#include <ranges>
#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {

using State = AbstractMachineLivenessAnalysis::State;

State AbstractMachineLivenessAnalysis::Transfer(State state, Instruction inst) {
  if (auto reg = GetTargetRegister(inst); reg.has_value()) {
    state.live_in.Remove(*reg);
  }
  for (auto reg : GetSourceRegisters(inst)) {
    state.live_in.Insert(reg);
  }
  return state;
}

AbstractMachineLivenessAnalysis::AbstractMachineLivenessAnalysis(
    const AbstractMachineControlFlowGraph& am_cfg)
    : am_cfg_(am_cfg) {}

State AbstractMachineLivenessAnalysis::Transfer(
    std::optional<State> prior_state,
    const AbstractMachineControlFlowGraph::BlockRef& block_ref) {
  State state;
  if (prior_state.has_value()) {
    state.live_out = std::move(prior_state->live_in);
    state.live_in = state.live_out;
  }

  const auto& block = am_cfg_.get(block_ref);
  for (auto inst : block.instructions | std::views::reverse) {
    state = Transfer(std::move(state), inst);
  }
  for (const auto& phi : block.phis) {
    state.live_in.Remove(phi.target);
    for (const auto& source : phi.sources) state.live_in.Insert(source);
  }

  return state;
}

State AbstractMachineLivenessAnalysis::Join(State left, State right) {
  State state;
  state.live_in = std::move(left.live_in);
  for (RegId reg : right.live_in) state.live_in.Insert(reg);
  return state;
}

}  // namespace lucid
