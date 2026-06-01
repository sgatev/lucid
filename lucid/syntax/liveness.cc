#include "lucid/syntax/liveness.h"

#include <cassert>
#include <optional>
#include <ranges>
#include <utility>
#include <variant>

#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {

using State = SyntaxLivenessAnalysis::State;

State SyntaxLivenessAnalysis::Transfer(
    State state, const SyntaxControlFlowGraph::Sequence& seq) {
  if (seq.stmt.has_value()) {
    if (const auto* var_decl_stmt =
            std::get_if<VarDeclStmt>(&sctx_.DerefStmt(*seq.stmt))) {
      state.live_in.Remove(var_decl_stmt->name);
    } else if (const auto* var_assign_stmt =
                   std::get_if<VarAssignStmt>(&sctx_.DerefStmt(*seq.stmt))) {
      state.live_in.Remove(var_assign_stmt->name);
    }
  }
  for (ExprRef expr_ref : seq.expressions) {
    if (const auto* ident_expr =
            std::get_if<IdentExpr>(&sctx_.DerefExpr(expr_ref))) {
      state.live_in.Insert(ident_expr->name);
    }
  }
  return state;
}

SyntaxLivenessAnalysis::SyntaxLivenessAnalysis(
    const SyntaxContext& sctx, const SyntaxControlFlowGraph& scfg)
    : sctx_(sctx), scfg_(scfg) {}

State SyntaxLivenessAnalysis::Transfer(
    std::optional<State> prior_state,
    const SyntaxControlFlowGraph::BlockRef& block_ref) {
  const auto& block = scfg_.get(block_ref);
  State state;

  if (prior_state.has_value()) {
    state.live_out = std::move(prior_state->live_in);

    for (const auto& next_block_ref : block.next) {
      const auto& next_block = scfg_.get(next_block_ref);
      for (const auto& phi_ref : next_block.phis) {
        const auto& phi = scfg_.deref(phi_ref);
        for (int i = 0; i < next_block.preds.size(); ++i) {
          if (next_block.preds[i] == block.ref) {
            state.live_out.Insert(phi.args[i]);
            break;
          }
        }
      }
    }

    state.live_in = state.live_out;
  }

  for (const auto& seq : block.sequences | std::views::reverse) {
    state = Transfer(std::move(state), seq);
  }

  for (const auto& phi_ref : block.phis) {
    const auto& phi = scfg_.deref(phi_ref);
    state.live_in.Remove(phi.name);
  }

  return state;
}

State SyntaxLivenessAnalysis::Join(State left, State right) {
  State state;
  state.live_in = std::move(left.live_in);
  for (StringIndex::Ref var : right.live_in) state.live_in.Insert(var);
  return state;
}

}  // namespace lucid
