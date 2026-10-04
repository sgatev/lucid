#include "lucid/syntax/liveness.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/core/container/hash_set.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/context.h"

namespace lucid {

using State = SyntaxLivenessAnalysis::State;

void SyntaxLivenessAnalysis::Transfer(
    State& state, const SyntaxControlFlowGraph::Sequence& seq) {
  if (seq.stmt.has_value()) {
    if (const auto* var_decl_stmt =
            std::get_if<VarDeclStmt>(&sctx_.DerefStmt(*seq.stmt))) {
      Remove(state, var_decl_stmt->name);
    } else if (const auto* var_assign_stmt =
                   std::get_if<VarAssignStmt>(&sctx_.DerefStmt(*seq.stmt))) {
      Remove(state, var_assign_stmt->name);
    }
  }
  for (ExprRef expr_ref : seq.expressions) {
    const auto* ident_expr = std::get_if<IdentExpr>(&sctx_.DerefExpr(expr_ref));
    if (ident_expr != nullptr) Insert(state, ident_expr->name);
  }
}

SyntaxLivenessAnalysis::SyntaxLivenessAnalysis(
    const SyntaxContext& sctx, const SyntaxControlFlowGraph& scfg)
    : sctx_(sctx), scfg_(scfg) {
  // A variable is only ever made live where it is read, so the ones read are
  // all there is to work out.
  for (const auto& block : scfg_.blocks()) {
    for (const auto& phi_ref : block.phis) {
      for (StringIndex::Ref arg : scfg_.deref(phi_ref).args) Track(arg);
    }
    for (const auto& seq : block.sequences) {
      for (ExprRef expr_ref : seq.expressions) {
        const auto* ident_expr =
            std::get_if<IdentExpr>(&sctx_.DerefExpr(expr_ref));
        if (ident_expr != nullptr) Track(ident_expr->name);
      }
    }
  }
}

SyntaxLivenessAnalysis::SyntaxLivenessAnalysis(
    const SyntaxContext& sctx, const SyntaxControlFlowGraph& scfg,
    const HashSet<StringIndex::Ref>& vars)
    : sctx_(sctx), scfg_(scfg) {
  for (StringIndex::Ref var : vars) Track(var);
}

State SyntaxLivenessAnalysis::Transfer(
    std::optional<State>&& prior_state,
    const SyntaxControlFlowGraph::BlockRef& block_ref) {
  State state;

  // What reaches the block is what is live where it exits, and the walk
  // backwards over it turns that into what is live where it enters.
  const auto& block = scfg_.get(block_ref);
  if (prior_state.has_value()) {
    state.live_in = std::move(prior_state->live_in);
  }
  // Every state holds a word for every 64 variables worked out, so that two
  // holding the same variables compare equal.
  state.live_in.resize((vars_.size() + kWordBits - 1) / kWordBits);
  if (prior_state.has_value()) {
    for (const auto& succ_ref : block.succs) {
      const auto& succ_block = scfg_.get(succ_ref);
      for (const auto& phi_ref : succ_block.phis) {
        const auto& phi = scfg_.deref(phi_ref);
        // An argument is paired with the predecessor it comes from, which is
        // this block at whichever position it holds among them.
        for (int i = 0; i < succ_block.preds.size(); ++i) {
          if (succ_block.preds[i] == block.ref) {
            Insert(state, phi.args[i]);
            break;
          }
        }
      }
    }
  }
  for (const auto& seq : block.sequences | std::views::reverse) {
    Transfer(state, seq);
  }
  for (const auto& phi_ref : block.phis) {
    const auto& phi = scfg_.deref(phi_ref);
    Remove(state, phi.name);
  }

  return state;
}

void SyntaxLivenessAnalysis::Join(State& left, const State& right) {
  // What a join starts from holds no words at all.
  if (left.live_in.empty()) {
    left.live_in = right.live_in;
    return;
  }
  assert(left.live_in.size() == right.live_in.size());
  for (std::size_t i = 0; i < right.live_in.size(); ++i) {
    left.live_in[i] |= right.live_in[i];
  }
}

bool SyntaxLivenessAnalysis::IsLiveIn(const State& state,
                                      StringIndex::Ref var) const {
  const auto bit = bits_.Get(var);
  if (!bit.has_value()) return false;
  return (state.live_in[*bit / kWordBits] >> (*bit % kWordBits) & 1) != 0;
}

std::vector<StringIndex::Ref> SyntaxLivenessAnalysis::LiveIn(
    const State& state) const {
  std::vector<StringIndex::Ref> live;
  for (std::size_t bit = 0; bit < vars_.size(); ++bit) {
    if ((state.live_in[bit / kWordBits] >> (bit % kWordBits) & 1) != 0) {
      live.push_back(vars_[bit]);
    }
  }
  return live;
}

void SyntaxLivenessAnalysis::Track(StringIndex::Ref var) {
  if (bits_.Insert(var, static_cast<std::uint32_t>(vars_.size()))) {
    vars_.push_back(var);
  }
}

void SyntaxLivenessAnalysis::Insert(State& state, StringIndex::Ref var) const {
  const auto bit = bits_.Get(var);
  if (!bit.has_value()) return;
  state.live_in[*bit / kWordBits] |= std::uint64_t{1} << (*bit % kWordBits);
}

void SyntaxLivenessAnalysis::Remove(State& state, StringIndex::Ref var) const {
  const auto bit = bits_.Get(var);
  if (!bit.has_value()) return;
  state.live_in[*bit / kWordBits] &= ~(std::uint64_t{1} << (*bit % kWordBits));
}

}  // namespace lucid
