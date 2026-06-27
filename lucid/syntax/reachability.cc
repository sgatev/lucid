#include "lucid/syntax/reachability.h"

#include <cassert>
#include <optional>
#include <utility>

#include "lucid/syntax/cfg.h"
#include "lucid/syntax/context.h"

namespace lucid {

using State = SyntaxReachabilityAnalysis::State;

SyntaxReachabilityAnalysis::SyntaxReachabilityAnalysis(
    SyntaxControlFlowGraph& s_cfg, SyntaxContext& ctx)
    : s_cfg_(s_cfg), ctx_(ctx) {}

State SyntaxReachabilityAnalysis::Transfer(
    std::optional<State>&& prior_state,
    const SyntaxControlFlowGraph::BlockRef& block_ref) {
  State state;
  if (prior_state.has_value()) {
    state.vars_in = std::move(prior_state->vars_out);
    state.vars_out = state.vars_in;
  }

  if (block_ref == s_cfg_.first) {
    for (auto param_ref : s_cfg_.func_params) {
      const auto& param = ctx_.DerefParam(param_ref);
      state.vars_in.Set(param.name, param_ref);
      state.vars_out.Set(param.name, param_ref);
    }
  }

  auto& block = s_cfg_.get(block_ref);
  for (auto phi_ref : block.phis) {
    const auto& phi = s_cfg_.deref(phi_ref);
    state.vars_out.Set(phi.name, phi_ref);
  }
  for (auto& seq : block.sequences) {
    if (!seq.stmt.has_value()) continue;

    auto& stmt = ctx_.DerefStmt(*seq.stmt);
    if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
      state.vars_out.Set(var_decl_stmt->name, *seq.stmt);
    } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
      state.vars_out.Set(var_assign_stmt->name, *seq.stmt);
    }
  }

  return state;
}

State SyntaxReachabilityAnalysis::Join(State&& left, const State& right) {
  State state;
  state.vars_out = std::move(left.vars_out);
  for (auto [from, to] : right.vars_out) state.vars_out.Set(from, to);
  return state;
}

}  // namespace lucid
