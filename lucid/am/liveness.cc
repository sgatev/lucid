#include "lucid/am/liveness.h"

#include <cassert>
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

State AbstractMachineLivenessAnalysis::MakeInitial() { return {}; }

State AbstractMachineLivenessAnalysis::Transfer(
    State state, const AbstractMachineControlFlowGraph::BlockRef& block_ref) {
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
  for (RegId reg : left.live_in) state.live_out.Insert(reg);
  for (RegId reg : right.live_in) state.live_out.Insert(reg);
  state.live_in = state.live_out;
  return state;
}

}  // namespace lucid
