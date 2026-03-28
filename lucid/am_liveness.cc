#include "lucid/am_liveness.h"

#include <ranges>

#include "lucid/am.h"
#include "lucid/am_cfg.h"

namespace lucid {

using State = AbstractMachineLivenessAnalysis::State;

AbstractMachineLivenessAnalysis::AbstractMachineLivenessAnalysis(
    const AbstractMachineControlFlowGraph& am_cfg)
    : am_cfg_(am_cfg) {}

State AbstractMachineLivenessAnalysis::MakeInitial() { return {}; }

State AbstractMachineLivenessAnalysis::Transfer(
    State state, const AbstractMachineControlFlowGraph::BlockRef& block) {
  for (auto inst : am_cfg_.get(block).instructions | std::views::reverse) {
    if (auto* cinst = std::get_if<MoveReg32>(&inst)) {
      state.live_in.Insert(cinst->src_reg);
      state.live_in.Remove(cinst->dst_reg);
    } else if (auto* cinst = std::get_if<MoveReg64>(&inst)) {
      state.live_in.Insert(cinst->src_reg);
      state.live_in.Remove(cinst->dst_reg);
    } else if (auto* cinst = std::get_if<SetReg32>(&inst)) {
      state.live_in.Remove(cinst->dst_reg);
    } else if (auto* cinst = std::get_if<SetReg64>(&inst)) {
      state.live_in.Remove(cinst->dst_reg);
    } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
      state.live_in.Remove(cinst->dst_reg);
    } else if (auto* cinst = std::get_if<AddReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<AddReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<SubReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<SubReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<MulReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<MulReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<DivReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<DivReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<ModReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
    } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
      state.live_in.Insert(cinst->lhs_reg);
      state.live_in.Insert(cinst->rhs_reg);
      state.live_in.Remove(cinst->res_reg);
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
      state.live_in.Insert(cinst->offset_reg);
      state.live_in.Remove(cinst->dst_reg);
    } else if (auto* cinst = std::get_if<LoadStack64>(&inst)) {
      state.live_in.Remove(cinst->dst_reg);
    } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
      state.live_in.Insert(cinst->offset_reg);
      state.live_in.Remove(cinst->dst_reg);
    }
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
