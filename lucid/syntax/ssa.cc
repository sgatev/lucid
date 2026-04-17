#include "lucid/syntax/ssa.h"

#include <cassert>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "lucid/core/container/graph/dominator.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/reachability.h"

namespace lucid {
namespace {

using BlockRef = SyntaxControlFlowGraph::BlockRef;
using Phi = SyntaxControlFlowGraph::Phi;
using PhiRef = SyntaxControlFlowGraph::PhiRef;

std::unordered_map<StringIndex::Ref, std::pair<TypeRef, HashSet<BlockRef>>>
CollectVarDefs(const SyntaxContext& ctx, const SyntaxControlFlowGraph& scfg) {
  std::unordered_map<StringIndex::Ref, std::pair<TypeRef, HashSet<BlockRef>>>
      defs;
  for (auto param_ref : scfg.func_params) {
    const auto& param = ctx.DerefParam(param_ref);
    defs[param.name].first = param.type_constraint;
    defs[param.name].second.Insert(scfg.first);
  }
  for (const auto& block : scfg.blocks()) {
    for (const auto& seq : block.sequences) {
      if (!seq.stmt.has_value()) continue;

      const auto& stmt = ctx.DerefStmt(*seq.stmt);
      if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
        defs[var_decl_stmt->name].first = var_decl_stmt->type_constraint;
        defs[var_decl_stmt->name].second.Insert(block.ref);
      } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
        defs[var_assign_stmt->name].second.Insert(block.ref);
      }
    }
  }
  return defs;
}

void InitPhiFunctions(const SyntaxContext& ctx, SyntaxControlFlowGraph& scfg) {
  const std::vector<std::optional<BlockRef>> idoms =
      ComputeImmediateDominators(scfg);
  const std::unordered_map<BlockRef, std::unordered_set<BlockRef>> dom_fronts =
      ComputeDominanceFrontiers(scfg, idoms);
  const std::unordered_map<StringIndex::Ref,
                           std::pair<TypeRef, HashSet<BlockRef>>>
      var_defs = CollectVarDefs(ctx, scfg);

  for (const auto& [var, add] : var_defs) {
    const auto& [type, def_blocks] = add;

    if (def_blocks.size() < 2) continue;

    HashSet<BlockRef> visited;
    HashSet<BlockRef> pending = def_blocks;

    while (!pending.empty()) {
      auto block = *pending.begin();
      pending.Remove(block);

      auto dom_front_it = dom_fronts.find(block);
      if (dom_front_it == dom_fronts.end()) continue;

      for (auto y : dom_front_it->second) {
        if (visited.Contains(y)) continue;

        auto& yb = scfg.get(y);

        Phi phi = {
            .name = var,
            .type_constraint = type,
        };
        for (const auto& _ : yb.preds) {
          phi.args.push_back(var);
        }

        yb.phis.push_back(scfg.add(std::move(phi)));
        visited.Insert(y);

        if (!def_blocks.Contains(y)) pending.Insert(y);
      }
    }
  }
}

void RenameVariables(SyntaxContext& ctx, SyntaxControlFlowGraph& scfg) {
  SyntaxReachabilityAnalysis reachability_analysis(scfg, ctx);
  std::vector<std::optional<SyntaxReachabilityAnalysis::State>>
      reachability_block_states =
          RunDataflow(Forward(scfg), reachability_analysis);

  HashMap<SyntaxReachabilityAnalysis::NamedValueSource, StringIndex::Ref>
      renames;
  for (auto param_ref : scfg.func_params) {
    auto& param = ctx.DerefParam(param_ref);

    const auto new_name = ctx.AddUniqueIdent();
    renames.Set(param_ref, new_name);

    param.name = new_name;
  }

  std::vector<SyntaxControlFlowGraph::BlockRef> block_refs = Vertices(scfg);
  std::sort(block_refs.begin(), block_refs.end(),
            CompareReversePostOrder(scfg));

  for (auto block_ref : block_refs) {
    auto& block = scfg.get(block_ref);
    auto reachability_block_state = reachability_block_states[block.ref.id()];
    if (!reachability_block_state.has_value()) continue;

    for (auto phi_ref : block.phis) {
      auto& phi = scfg.deref(phi_ref);

      reachability_block_state->vars_in.Set(phi.name, phi_ref);

      const auto new_name = ctx.AddUniqueIdent();
      renames.Set(phi_ref, new_name);

      phi.name = new_name;
    }
    for (auto& seq : block.sequences) {
      for (auto expr_ref : seq.expressions) {
        auto& expr = ctx.DerefExpr(expr_ref);
        auto* ident_expr = std::get_if<IdentExpr>(&expr);
        if (ident_expr == nullptr) continue;

        const auto nvs =
            reachability_block_state->vars_in.Find(ident_expr->name);
        assert(nvs.has_value());

        const auto new_name = renames.Find(*nvs);
        assert(new_name.has_value());

        ident_expr->name = *new_name;
      }
      if (seq.stmt.has_value()) {
        auto& stmt = ctx.DerefStmt(*seq.stmt);
        if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
          reachability_block_state->vars_in.Set(var_decl_stmt->name, *seq.stmt);

          const auto new_name = ctx.AddUniqueIdent();
          renames.Set(*seq.stmt, new_name);

          var_decl_stmt->name = new_name;
        } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
          reachability_block_state->vars_in.Set(var_assign_stmt->name,
                                                *seq.stmt);

          const auto new_name = ctx.AddUniqueIdent();
          renames.Set(*seq.stmt, new_name);

          seq.stmt = ctx.Add(VarDeclStmt{
              .name = new_name,
              .type_constraint = GetType(ctx.DerefExpr(var_assign_stmt->expr)),
              .init = var_assign_stmt->expr,
          });
        }
      }
    }
  }
  for (auto& block : scfg.blocks()) {
    for (auto phi_ref : block.phis) {
      auto& phi = scfg.deref(phi_ref);
      for (int i = 0; i < phi.args.size(); ++i) {
        const auto& reachability_block_state =
            reachability_block_states[block.preds[i].id()];
        assert(reachability_block_state.has_value());

        const auto nvs = reachability_block_state->vars_out.Find(phi.args[i]);
        assert(nvs.has_value());

        const auto new_name = renames.Find(*nvs);
        assert(new_name.has_value());

        phi.args[i] = *new_name;
      }
    }
  }
}

}  // namespace

void ConvertToStaticSingleAssignment(SyntaxContext& ctx,
                                     SyntaxControlFlowGraph& scfg) {
  InitPhiFunctions(ctx, scfg);
  RenameVariables(ctx, scfg);
}

}  // namespace lucid
