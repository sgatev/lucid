#include "lucid/am/liveness.h"

#include <cassert>
#include <ranges>
#include <utility>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {

using State = AbstractMachineLivenessAnalysis::State;

State AbstractMachineLivenessAnalysis::Transfer(State state, Instruction inst) {
  if (std::holds_alternative<PushStack>(inst) ||
      std::holds_alternative<PopStack>(inst) ||
      std::holds_alternative<Label>(inst) ||
      std::holds_alternative<Jump>(inst) ||
      std::holds_alternative<UncondJump>(inst)) {
    // Nothing to do here.
  } else if (auto* cinst = std::get_if<MoveReg32>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
    state.live_in.Insert(cinst->src_reg);
  } else if (auto* cinst = std::get_if<MoveReg64>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
    state.live_in.Insert(cinst->src_reg);
  } else if (auto* cinst = std::get_if<SetReg32>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
  } else if (auto* cinst = std::get_if<SetReg64>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
  } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
  } else if (auto* cinst = std::get_if<AddReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<AddReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<SubReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<SubReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<MulReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<MulReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<DivReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<DivReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<ModReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
    state.live_in.Remove(cinst->res_reg);
    state.live_in.Insert(cinst->lhs_reg);
    state.live_in.Insert(cinst->rhs_reg);
  } else if (auto* cinst = std::get_if<StoreStack32>(&inst)) {
    state.live_in.Insert(cinst->src_reg);
  } else if (auto* cinst = std::get_if<StoreStackReg32>(&inst)) {
    state.live_in.Insert(cinst->src_reg);
    state.live_in.Insert(cinst->offset_reg);
  } else if (auto* cinst = std::get_if<StoreStack64>(&inst)) {
    state.live_in.Insert(cinst->src_reg);
  } else if (auto* cinst = std::get_if<StoreStackReg64>(&inst)) {
    state.live_in.Insert(cinst->src_reg);
    state.live_in.Insert(cinst->offset_reg);
  } else if (auto* cinst = std::get_if<LoadStack32>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
  } else if (auto* cinst = std::get_if<LoadStackReg32>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
    state.live_in.Insert(cinst->offset_reg);
  } else if (auto* cinst = std::get_if<LoadStack64>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
  } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
    state.live_in.Remove(cinst->dst_reg);
    state.live_in.Insert(cinst->offset_reg);
  } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
    state.live_in.Remove(cinst->res.reg);
    for (const auto& arg : cinst->args) state.live_in.Insert(arg.reg);
  } else if (auto* cinst = std::get_if<CondJump>(&inst)) {
    state.live_in.Insert(cinst->cond_reg);
  } else if (auto* cinst = std::get_if<Return>(&inst)) {
    state.live_in.Insert(cinst->res_reg);
  } else {
    assert(false && "unhandled instruction type");
  }
  return state;
}

AbstractMachineLivenessAnalysis::AbstractMachineLivenessAnalysis(
    const AbstractMachineControlFlowGraph& am_cfg)
    : am_cfg_(am_cfg) {}

State AbstractMachineLivenessAnalysis::MakeInitial() { return {}; }

State AbstractMachineLivenessAnalysis::Transfer(
    State state, const AbstractMachineControlFlowGraph::BlockRef& block) {
  for (auto inst : am_cfg_.get(block).instructions | std::views::reverse) {
    state = Transfer(std::move(state), inst);
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
