#include "lucid/syntax/ssa.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "lucid/core/container/graph/dominator.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/liveness.h"

namespace lucid {
namespace {

using BlockRef = SyntaxControlFlowGraph::BlockRef;
using Phi = SyntaxControlFlowGraph::Phi;
using PhiRef = SyntaxControlFlowGraph::PhiRef;

HashMap<StringIndex::Ref, std::pair<TypeRef, HashSet<BlockRef>>> CollectVarDefs(
    const SyntaxContext& syn_ctx, const SyntaxControlFlowGraph& syn_cfg) {
  HashMap<StringIndex::Ref, std::pair<TypeRef, HashSet<BlockRef>>> defs;
  for (auto param_ref : syn_cfg.func_params) {
    const auto& param = syn_ctx.DerefParam(param_ref);
    auto& def = defs.Emplace(param.name);
    def.first = param.type_constraint;
    def.second.Insert(syn_cfg.first);
  }
  for (const auto& block : syn_cfg.blocks()) {
    for (const auto& seq : block.sequences) {
      if (!seq.stmt.has_value()) continue;

      const auto& stmt = syn_ctx.DerefStmt(*seq.stmt);
      if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
        auto& def = defs.Emplace(var_decl_stmt->name);
        def.first = var_decl_stmt->type_constraint;
        def.second.Insert(block.ref);
      } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
        defs.Emplace(var_assign_stmt->name).second.Insert(block.ref);
      }
    }
  }
  return defs;
}

void InitPhiFunctions(const SyntaxContext& syn_ctx,
                      SyntaxControlFlowGraph& syn_cfg,
                      const std::vector<std::optional<BlockRef>>& idoms) {
  SyntaxLivenessAnalysis liveness_analysis(syn_ctx, syn_cfg);
  std::vector<std::optional<SyntaxLivenessAnalysis::State>>
      liveness_block_states = RunDataflow(Backward(syn_cfg), liveness_analysis);
  const HashMap<BlockRef, HashSet<BlockRef>> dom_fronts =
      ComputeDominanceFrontiers(syn_cfg, idoms);
  const HashMap<StringIndex::Ref, std::pair<TypeRef, HashSet<BlockRef>>>
      var_defs = CollectVarDefs(syn_ctx, syn_cfg);

  for (const auto& [var, add] : var_defs) {
    const auto& [type, def_blocks] = add;

    if (def_blocks.size() < 2) continue;

    HashSet<BlockRef> visited;

    std::vector<BlockRef> pending;
    pending.reserve(def_blocks.size());
    for (BlockRef def_block : def_blocks) pending.push_back(def_block);

    while (!pending.empty()) {
      const BlockRef block = pending.back();
      pending.pop_back();

      auto dom_front_it = dom_fronts.Get(block);
      if (!dom_front_it.has_value()) continue;

      for (auto y : *dom_front_it) {
        if (visited.Contains(y)) continue;

        if (!liveness_block_states[y.id()]->live_in.Contains(var)) continue;

        auto& yb = syn_cfg.get(y);

        Phi phi = {
            .name = var,
            .type_constraint = type,
        };
        for (const auto& _ : yb.preds) {
          phi.args.push_back(var);
        }

        yb.phis.push_back(syn_cfg.add(std::move(phi)));
        visited.Insert(y);

        if (!def_blocks.Contains(y)) pending.push_back(y);
      }
    }
  }
}

// The names the variables of a function go by as it is walked, and what to
// put back as the walk leaves the block that gave them those names.
//
// A definition holds from where it is made until the walk leaves the block
// that dominates it, which is what makes the name in force at a use the one
// that reaches it.
class RenameScope {
 public:
  // Records that `from` goes by `to` from here on.
  void Define(StringIndex::Ref from, StringIndex::Ref to) {
    const auto in_force = names_.Get(from);
    undo_.emplace_back(from, in_force.has_value()
                                 ? std::optional<StringIndex::Ref>(*in_force)
                                 : std::nullopt);
    names_.Set(from, to);
  }

  // Returns the name `from` goes by, which is the definition reaching here.
  StringIndex::Ref InForce(StringIndex::Ref from) const {
    const auto to = names_.Get(from);
    assert(to.has_value());
    return *to;
  }

  // Returns a mark that `UndoTo` takes the names back to.
  std::size_t Mark() const { return undo_.size(); }

  // Takes back every definition made since `mark`.
  void UndoTo(std::size_t mark) {
    while (undo_.size() > mark) {
      const auto& [from, in_force] = undo_.back();
      if (in_force.has_value()) {
        names_.Set(from, *in_force);
      } else {
        names_.Remove(from);
      }
      undo_.pop_back();
    }
  }

 private:
  HashMap<StringIndex::Ref, StringIndex::Ref> names_;
  std::vector<std::pair<StringIndex::Ref, std::optional<StringIndex::Ref>>>
      undo_;
};

// Returns the blocks each block is the immediate dominator of, indexed by
// block id.
//
// A block the analysis never reached has no immediate dominator and is left
// out, along with everything below it.
std::vector<std::vector<BlockRef>> CollectDominatorChildren(
    const SyntaxControlFlowGraph& syn_cfg,
    const std::vector<std::optional<BlockRef>>& idoms) {
  std::vector<std::vector<BlockRef>> children(idoms.size());
  for (const auto& block : syn_cfg.blocks()) {
    if (block.ref == syn_cfg.first) continue;

    const auto& idom = idoms[block.ref.id()];
    if (!idom.has_value()) continue;

    children[idom->id()].push_back(block.ref);
  }
  return children;
}

// Renames the variables of one block, and the arguments that the phis of the
// blocks after it take from it.
void RenameBlock(SyntaxContext& syn_ctx, SyntaxControlFlowGraph& syn_cfg,
                 BlockRef block_ref, RenameScope& scope) {
  auto& block = syn_cfg.get(block_ref);

  // A phi defines its variable where the block is entered, before anything in
  // the block reads it.
  for (auto phi_ref : block.phis) {
    auto& phi = syn_cfg.deref(phi_ref);

    const auto new_name = syn_ctx.AddUniqueIdent();
    scope.Define(phi.name, new_name);
    phi.name = new_name;
  }

  for (auto& seq : block.sequences) {
    // The expressions of a sequence read what reaches it, which is what the
    // statement of that sequence then defines over.
    for (auto expr_ref : seq.expressions) {
      auto& expr = syn_ctx.DerefExpr(expr_ref);
      auto* ident_expr = std::get_if<IdentExpr>(&expr);
      if (ident_expr == nullptr) continue;

      ident_expr->name = scope.InForce(ident_expr->name);
    }

    if (!seq.stmt.has_value()) continue;

    auto& stmt = syn_ctx.DerefStmt(*seq.stmt);
    if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
      const auto new_name = syn_ctx.AddUniqueIdent();
      scope.Define(var_decl_stmt->name, new_name);

      var_decl_stmt->name = new_name;
    } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
      const auto new_name = syn_ctx.AddUniqueIdent();
      scope.Define(var_assign_stmt->name, new_name);

      seq.stmt = syn_ctx.Add(VarDeclStmt{
          .name = new_name,
          .type_constraint = GetType(syn_ctx.DerefExpr(var_assign_stmt->expr)),
          .init = var_assign_stmt->expr,
      });
    } else if (auto* array_assign_stmt = std::get_if<ArrayAssignStmt>(&stmt)) {
      array_assign_stmt->name = scope.InForce(array_assign_stmt->name);
    }
  }

  // A phi in a block after this one takes its argument from the name in force
  // where this block leaves off. The argument still carries the name the
  // variable had in the source, which is what says where to look for it.
  for (std::size_t i = 0; i < block.succs.size(); ++i) {
    const BlockRef succ_ref = block.succs[i];

    // A block reached by more than one edge from this one is named more than
    // once among the successors, and is taken care of by the first of them.
    if (std::find(block.succs.begin(), block.succs.begin() + i, succ_ref) !=
        block.succs.begin() + i) {
      continue;
    }

    auto& succ_block = syn_cfg.get(succ_ref);
    for (std::size_t pred = 0; pred < succ_block.preds.size(); ++pred) {
      if (succ_block.preds[pred] != block_ref) continue;

      for (auto phi_ref : succ_block.phis) {
        auto& phi = syn_cfg.deref(phi_ref);
        phi.args[pred] = scope.InForce(phi.args[pred]);
      }
    }
  }
}

void RenameVariables(SyntaxContext& syn_ctx, SyntaxControlFlowGraph& syn_cfg,
                     const std::vector<std::optional<BlockRef>>& idoms) {
  RenameScope scope;

  // The parameters are what reaches the first block, and nothing takes them
  // back: they are in force over the whole function.
  for (auto param_ref : syn_cfg.func_params) {
    auto& param = syn_ctx.DerefParam(param_ref);

    const auto new_name = syn_ctx.AddUniqueIdent();
    scope.Define(param.name, new_name);

    param.name = new_name;
  }

  const std::vector<std::vector<BlockRef>> children =
      CollectDominatorChildren(syn_cfg, idoms);

  // A block is renamed before the blocks it dominates and has its names taken
  // back after them, which is a walk down the dominator tree and back up it.
  // The walk carries its own stack rather than the call stack, because a
  // dominator tree is as deep as a function is long.
  struct Pending {
    BlockRef block;
    std::size_t mark;
    bool renamed;
  };
  std::vector<Pending> pending;
  pending.push_back({syn_cfg.first, scope.Mark(), false});

  while (!pending.empty()) {
    if (pending.back().renamed) {
      scope.UndoTo(pending.back().mark);
      pending.pop_back();
      continue;
    }
    pending.back().renamed = true;

    const BlockRef block_ref = pending.back().block;
    RenameBlock(syn_ctx, syn_cfg, block_ref, scope);

    const std::size_t mark = scope.Mark();
    for (BlockRef child : children[block_ref.id()]) {
      pending.push_back({child, mark, false});
    }
  }
}

}  // namespace

void ConvertToStaticSingleAssignment(SyntaxContext& syn_ctx,
                                     SyntaxControlFlowGraph& syn_cfg) {
  // Placing the phi functions and renaming the variables both walk the same
  // dominator tree, so it is worked out once for the two of them. Placing a
  // phi function adds nothing to a block but the phi, which leaves the tree
  // as it was.
  const std::vector<std::optional<BlockRef>> idoms =
      ComputeImmediateDominators(syn_cfg);

  InitPhiFunctions(syn_ctx, syn_cfg, idoms);
  RenameVariables(syn_ctx, syn_cfg, idoms);
}

}  // namespace lucid
