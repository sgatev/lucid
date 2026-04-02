#include "lucid/liveness.h"

#include <ranges>
#include <variant>

#include "lucid/cfg.h"
#include "lucid/syntax/ast.h"

namespace lucid {

using State = LivenessAnalysis::State;

LivenessAnalysis::LivenessAnalysis(const SyntaxContext& ctx) : ctx_(ctx) {}

State LivenessAnalysis::MakeInitial() { return {}; }

State LivenessAnalysis::Transfer(State state,
                                 const ControlFlowGraph::Sequence& seq) {
  for (auto expr : seq.expressions | std::views::reverse) {
    if (auto* ident = std::get_if<IdentExpr>(&ctx_.DerefExpr(expr))) {
      state.live_in.Insert(ident->name);
    }
  }
  if (seq.stmt.has_value()) {
    if (auto* var_assign =
            std::get_if<VarAssignStmt>(&ctx_.DerefStmt(*seq.stmt))) {
      state.live_in.Remove(var_assign->name);
    } else if (auto* var_decl =
                   std::get_if<VarDeclStmt>(&ctx_.DerefStmt(*seq.stmt))) {
      state.live_in.Remove(var_decl->name);
    }
  }
  return state;
}

State LivenessAnalysis::Join(State left, State right) {
  State state;
  for (StringIndex::Ref ref : left.live_in) state.live_out.Insert(ref);
  for (StringIndex::Ref ref : right.live_in) state.live_out.Insert(ref);
  state.live_in = state.live_out;
  return state;
}

}  // namespace lucid
