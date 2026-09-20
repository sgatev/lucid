#include "lucid/syntax/ssa.h"

#include <cassert>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "lucid/core/container/graph/dominator.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/liveness.h"
#include "lucid/syntax/reachability.h"

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
                      SyntaxControlFlowGraph& syn_cfg) {
  SyntaxLivenessAnalysis liveness_analysis(syn_ctx, syn_cfg);
  std::vector<std::optional<SyntaxLivenessAnalysis::State>>
      liveness_block_states = RunDataflow(Backward(syn_cfg), liveness_analysis);
  const std::vector<std::optional<BlockRef>> idoms =
      ComputeImmediateDominators(syn_cfg);
  const HashMap<BlockRef, HashSet<BlockRef>> dom_fronts =
      ComputeDominanceFrontiers(syn_cfg, idoms);
  const HashMap<StringIndex::Ref, std::pair<TypeRef, HashSet<BlockRef>>>
      var_defs = CollectVarDefs(syn_ctx, syn_cfg);

  for (const auto& [var, add] : var_defs) {
    const auto& [type, def_blocks] = add;

    if (def_blocks.size() < 2) continue;

    HashSet<BlockRef> visited;
    HashSet<BlockRef> pending = def_blocks;

    while (!pending.empty()) {
      auto block = *pending.begin();
      pending.Remove(block);

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

        if (!def_blocks.Contains(y)) pending.Insert(y);
      }
    }
  }
}

void RenameVariables(SyntaxContext& syn_ctx, SyntaxControlFlowGraph& syn_cfg) {
  SyntaxReachabilityAnalysis reachability_analysis(syn_cfg, syn_ctx);
  std::vector<std::optional<SyntaxReachabilityAnalysis::State>>
      reachability_block_states =
          RunDataflow(Forward(syn_cfg), reachability_analysis);

  HashMap<SyntaxReachabilityAnalysis::NamedValueSource, StringIndex::Ref>
      renames;
  for (auto param_ref : syn_cfg.func_params) {
    auto& param = syn_ctx.DerefParam(param_ref);

    const auto new_name = syn_ctx.AddUniqueIdent();
    renames.Set(param_ref, new_name);

    param.name = new_name;
  }

  std::vector<SyntaxControlFlowGraph::BlockRef> block_refs = Vertices(syn_cfg);
  std::sort(block_refs.begin(), block_refs.end(),
            CompareReversePostOrder(syn_cfg));

  for (auto block_ref : block_refs) {
    auto& block = syn_cfg.get(block_ref);
    auto& reachability_block_state = reachability_block_states[block.ref.id()];
    if (!reachability_block_state.has_value()) continue;

    // Taken rather than copied: no block is walked twice, and what reaches a
    // block is read nowhere else.
    auto vars_in = std::move(reachability_block_state->vars_in);

    for (auto phi_ref : block.phis) {
      auto& phi = syn_cfg.deref(phi_ref);

      vars_in.Set(phi.name, phi_ref);

      const auto new_name = syn_ctx.AddUniqueIdent();
      renames.Set(phi_ref, new_name);

      phi.name = new_name;
    }
    for (auto& seq : block.sequences) {
      for (auto expr_ref : seq.expressions) {
        auto& expr = syn_ctx.DerefExpr(expr_ref);
        auto* ident_expr = std::get_if<IdentExpr>(&expr);
        if (ident_expr == nullptr) continue;

        const auto nvs = vars_in.Get(ident_expr->name);
        assert(nvs.has_value());

        const auto new_name = renames.Get(*nvs);
        assert(new_name.has_value());

        ident_expr->name = *new_name;
      }
      if (seq.stmt.has_value()) {
        auto& stmt = syn_ctx.DerefStmt(*seq.stmt);
        if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
          vars_in.Set(var_decl_stmt->name, *seq.stmt);

          const auto new_name = syn_ctx.AddUniqueIdent();
          renames.Set(*seq.stmt, new_name);

          var_decl_stmt->name = new_name;
        } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
          vars_in.Set(var_assign_stmt->name, *seq.stmt);

          const auto new_name = syn_ctx.AddUniqueIdent();
          renames.Set(*seq.stmt, new_name);

          seq.stmt = syn_ctx.Add(VarDeclStmt{
              .name = new_name,
              .type_constraint =
                  GetType(syn_ctx.DerefExpr(var_assign_stmt->expr)),
              .init = var_assign_stmt->expr,
          });
        } else if (auto* array_assign_stmt =
                       std::get_if<ArrayAssignStmt>(&stmt)) {
          const auto nvs = vars_in.Get(array_assign_stmt->name);
          assert(nvs.has_value());

          const auto new_name = renames.Get(*nvs);
          assert(new_name.has_value());

          array_assign_stmt->name = *new_name;
        }
      }
    }
  }
  for (auto& block : syn_cfg.blocks()) {
    for (auto phi_ref : block.phis) {
      auto& phi = syn_cfg.deref(phi_ref);
      for (int i = 0; i < phi.args.size(); ++i) {
        const auto& reachability_block_state =
            reachability_block_states[block.preds[i].id()];
        assert(reachability_block_state.has_value());

        const auto nvs = reachability_block_state->vars_out.Get(phi.args[i]);
        assert(nvs.has_value());

        const auto new_name = renames.Get(*nvs);
        assert(new_name.has_value());

        phi.args[i] = *new_name;
      }
    }
  }
}

}  // namespace

void ConvertToStaticSingleAssignment(SyntaxContext& syn_ctx,
                                     SyntaxControlFlowGraph& syn_cfg) {
  InitPhiFunctions(syn_ctx, syn_cfg);
  RenameVariables(syn_ctx, syn_cfg);
}

}  // namespace lucid
