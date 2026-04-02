#include "lucid/ssa.h"

#include <cstddef>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "lucid/cfg.h"
#include "lucid/core/container/graph/dominator.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"

namespace lucid {
namespace {

using BlockRef = ControlFlowGraph::BlockRef;
using Phi = ControlFlowGraph::Phi;

std::unordered_map<StringIndex::Ref,
                   std::pair<TypeRef, std::unordered_set<BlockRef>>>
CollectVarDefs(const SyntaxContext& ctx, const ControlFlowGraph& cfg) {
  std::unordered_map<StringIndex::Ref,
                     std::pair<TypeRef, std::unordered_set<BlockRef>>>
      defs;
  for (auto param_ref : cfg.func_params) {
    const auto& param = ctx.DerefParam(param_ref);
    defs[param.name].first = param.type_constraint;
    defs[param.name].second.insert(cfg.first);
  }
  for (const auto& block : cfg.blocks()) {
    for (const auto& seq : block.sequences) {
      if (!seq.stmt.has_value()) continue;

      const auto& stmt = ctx.DerefStmt(*seq.stmt);
      if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
        defs[var_decl_stmt->name].first = var_decl_stmt->type_constraint;
        defs[var_decl_stmt->name].second.insert(block.ref);
      } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
        defs[var_assign_stmt->name].second.insert(block.ref);
      }
    }
  }
  return defs;
}

void InitPhiFunctions(const SyntaxContext& ctx, ControlFlowGraph& cfg) {
  const std::vector<std::optional<BlockRef>> idoms =
      ComputeImmediateDominators(cfg);
  const std::unordered_map<BlockRef, std::unordered_set<BlockRef>> dom_fronts =
      ComputeDominanceFrontiers(cfg, idoms);
  const std::unordered_map<StringIndex::Ref,
                           std::pair<TypeRef, std::unordered_set<BlockRef>>>
      var_defs = CollectVarDefs(ctx, cfg);

  for (const auto& [var, add] : var_defs) {
    const auto& [type, def_blocks] = add;

    if (def_blocks.size() < 2) continue;

    std::unordered_set<BlockRef> visited;
    std::unordered_set<BlockRef> pending = def_blocks;

    while (!pending.empty()) {
      auto block = *pending.begin();
      pending.erase(block);

      auto dom_front_it = dom_fronts.find(block);
      if (dom_front_it == dom_fronts.end()) continue;

      for (auto y : dom_front_it->second) {
        if (visited.contains(y)) continue;

        auto& yb = cfg.get(y);

        Phi phi = {
            .name = var,
            .type_constraint = type,
        };
        for (const auto& _ : yb.preds) {
          phi.args.push_back(var);
        }

        yb.phis.push_back(std::move(phi));
        visited.insert(y);

        if (!def_blocks.contains(y)) pending.insert(y);
      }
    }
  }
}

void RenameVariables(SyntaxContext& ctx, ControlFlowGraph& cfg) {
  std::vector<std::unordered_map<StringIndex::Ref, StringIndex::Ref>>
      block_defs(cfg.blocks().Size());

  for (auto param_ref : cfg.func_params) {
    auto& param = ctx.DerefParam(param_ref);
    StringIndex::Ref new_name = ctx.AddUniqueIdent();
    block_defs[cfg.first.id()].insert_or_assign(param.name, new_name);
    param.name = new_name;
  }

  std::vector<bool> visited(cfg.blocks().Size(), false);

  std::stack<BlockRef> pending;
  pending.push(cfg.first);

  while (!pending.empty()) {
    auto block_ref = pending.top();
    pending.pop();

    if (visited[block_ref.id()]) continue;
    visited[block_ref.id()] = true;

    auto& block = cfg.blocks().Get(block_ref);

    auto& reaching_defs = block_defs[block_ref.id()];
    for (auto pred : block.preds) {
      for (const auto& [k, v] : block_defs[pred.id()]) {
        reaching_defs.insert_or_assign(k, v);
      }
    }

    for (auto& phi : block.phis) {
      StringIndex::Ref new_name = ctx.AddUniqueIdent();
      reaching_defs.insert_or_assign(phi.name, new_name);
      phi.name = new_name;
    }

    for (auto& seq : block.sequences) {
      for (size_t i = 0; i < seq.expressions.size(); ++i) {
        Expr& expr = ctx.DerefExpr(seq.expressions[i]);
        auto* ident_expr = std::get_if<IdentExpr>(&expr);
        if (ident_expr == nullptr) continue;

        ident_expr->name = reaching_defs.at(ident_expr->name);
      }

      if (seq.stmt.has_value()) {
        auto& stmt = ctx.DerefStmt(*seq.stmt);
        if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
          StringIndex::Ref new_name = ctx.AddUniqueIdent();
          reaching_defs.insert_or_assign(var_decl_stmt->name, new_name);
          var_decl_stmt->name = new_name;
        } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
          StringIndex::Ref new_name = ctx.AddUniqueIdent();
          reaching_defs.insert_or_assign(var_assign_stmt->name, new_name);
          seq.stmt = ctx.Add(VarDeclStmt{
              .name = new_name,
              .type_constraint = GetType(ctx.DerefExpr(var_assign_stmt->expr)),
              .init = var_assign_stmt->expr,
          });
        }
      }
    }

    for (int i = block.next.size() - 1; i >= 0; --i) {
      if (!visited[block.next[i].id()]) pending.push(block.next[i]);
    }
  }

  for (ControlFlowGraph::Block& block : cfg.blocks()) {
    for (auto& phi : block.phis) {
      for (int j = 0; j < phi.args.size(); ++j) {
        phi.args[j] = block_defs[block.preds[j].id()].at(phi.args[j]);
      }
    }
  }
}

}  // namespace

void ConvertToStaticSingleAssignment(SyntaxContext& ctx,
                                     ControlFlowGraph& cfg) {
  InitPhiFunctions(ctx, cfg);
  RenameVariables(ctx, cfg);
}

void DestroyStaticSingleAssignment(SyntaxContext& ctx, ControlFlowGraph& cfg) {
  const std::vector<std::optional<BlockRef>> idoms =
      ComputeImmediateDominators(cfg);

  for (auto& block : cfg.blocks()) {
    if (block.phis.empty()) continue;

    assert(idoms[block.ref.id()].has_value());
    auto& dom = cfg.get(*idoms[block.ref.id()]);
    for (const auto& phi : block.phis) {
      auto& seq = dom.sequences.emplace_back();
      auto init_expr = IntLitExpr{.value = ctx.AddIdent("0")};
      init_expr.type = phi.type_constraint;
      auto init_expr_ref = ctx.Add(init_expr);
      seq.expressions.push_back(init_expr_ref);
      seq.stmt = ctx.Add(VarDeclStmt{
          .name = phi.name,
          .type_constraint = phi.type_constraint,
          .init = init_expr_ref,
      });

      for (int i = 0; i < block.preds.size(); ++i) {
        auto& pred = cfg.get(block.preds[i]);
        auto& seq = pred.sequences.emplace_back();
        auto assign_expr = IdentExpr{
            .name = phi.args[i],
        };
        assign_expr.type = phi.type_constraint;
        auto assign_expr_ref = ctx.Add(assign_expr);
        seq.expressions.push_back(assign_expr_ref);
        seq.stmt = ctx.Add(VarAssignStmt{
            .name = phi.name,
            .expr = assign_expr_ref,
        });
      }
    }
    block.phis.clear();
  }
}

}  // namespace lucid
