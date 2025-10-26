#include "lucid/ssa.h"

#include <cstddef>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/dom.h"

namespace lucid {
namespace {

using BlockRef = ControlFlowGraph::BlockRef;
using Phi = ControlFlowGraph::Phi;

std::unordered_map<std::string_view,
                   std::pair<TypeRef, std::unordered_set<BlockRef>>>
CollectVarDefs(const SyntaxContext& ctx, const ControlFlowGraph& cfg) {
  std::unordered_map<std::string_view,
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
  const std::vector<BlockRef> idoms = ComputeImmediateDominators(cfg);
  const std::unordered_map<BlockRef, std::unordered_set<BlockRef>> dom_fronts =
      ComputeDominanceFrontiers(cfg, idoms);
  const std::unordered_map<std::string_view,
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
            .name = std::string(var),
            .type_constraint = type,
        };
        for (const auto& _ : yb.preds) {
          phi.args.push_back(std::string(var));
        }

        yb.phis.push_back(std::move(phi));
        visited.insert(y);

        if (!def_blocks.contains(y)) pending.insert(y);
      }
    }
  }
}

void RenameVariables(SyntaxContext& ctx, ControlFlowGraph& cfg) {
  int counter = 0;

  std::vector<std::unordered_map<std::string, std::string>> block_defs(
      cfg.blocks().Size());

  for (auto param_ref : cfg.func_params) {
    auto& param = ctx.DerefParam(param_ref);
    std::string new_name = param.name + std::to_string(counter++);
    block_defs[cfg.first][param.name] = new_name;
    param.name = new_name;
  }

  std::vector<bool> visited(cfg.blocks().Size(), false);

  std::stack<BlockRef> pending;
  pending.push(cfg.first);

  while (!pending.empty()) {
    auto block_ref = pending.top();
    pending.pop();

    if (visited[block_ref]) continue;
    visited[block_ref] = true;

    auto& block = cfg.blocks().Get(block_ref);

    auto& reaching_defs = block_defs[block_ref];
    for (auto pred : block.preds) {
      for (const auto& [k, v] : block_defs[pred]) {
        reaching_defs[k] = v;
      }
    }

    for (auto& phi : block.phis) {
      std::string new_name = phi.name + std::to_string(counter++);
      reaching_defs[phi.name] = new_name;
      phi.name = new_name;
    }

    for (auto& seq : block.sequences) {
      for (size_t i = 0; i < seq.expressions.size(); ++i) {
        Expr& expr = ctx.DerefExpr(seq.expressions[i]);
        auto* ident_expr = std::get_if<IdentExpr>(&expr);
        if (ident_expr == nullptr) continue;

        ident_expr->name = reaching_defs[ident_expr->name];
      }

      if (seq.stmt.has_value()) {
        auto& stmt = ctx.DerefStmt(*seq.stmt);
        if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
          std::string new_name =
              std::string(var_decl_stmt->name) + std::to_string(counter++);
          reaching_defs[var_decl_stmt->name] = new_name;
          var_decl_stmt->name = new_name;
        } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
          std::string new_name =
              std::string(var_assign_stmt->name) + std::to_string(counter++);
          reaching_defs[var_assign_stmt->name] = new_name;
          seq.stmt = ctx.Add(VarDeclStmt{
              .name = new_name,
              .type_constraint = GetType(ctx.DerefExpr(var_assign_stmt->expr)),
              .init = var_assign_stmt->expr,
          });
        }
      }
    }

    for (int i = block.next.size() - 1; i >= 0; --i) {
      if (!visited[block.next[i]]) pending.push(block.next[i]);
    }
  }

  for (ControlFlowGraph::Block& block : cfg.blocks()) {
    for (auto& phi : block.phis) {
      for (int j = 0; j < phi.args.size(); ++j) {
        phi.args[j] = block_defs[block.preds[j]][phi.args[j]];
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
  const std::vector<BlockRef> idoms = ComputeImmediateDominators(cfg);

  for (auto& block : cfg.blocks()) {
    if (block.phis.empty()) continue;

    auto& dom = cfg.get(idoms[block.ref]);
    for (const auto& phi : block.phis) {
      auto& seq = dom.sequences.emplace_back();
      auto init_expr = ctx.Add(IntLitExpr{.value = "0"});
      seq.expressions.push_back(init_expr);
      seq.stmt = ctx.Add(VarDeclStmt{
          .name = phi.name,
          .type_constraint = phi.type_constraint,
          .init = init_expr,
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
