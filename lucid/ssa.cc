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
#include "lucid/dominators.h"

namespace lucid {
namespace {

using BlockRef = ControlFlowGraph::BlockRef;

std::unordered_map<std::string_view, std::unordered_set<BlockRef>>
CollectVarDefs(const SyntaxContext& ctx, const ControlFlowGraph& cfg) {
  std::unordered_map<std::string_view, std::unordered_set<BlockRef>> defs;
  for (const auto& block : cfg.blocks()) {
    for (const auto& seq : block.sequences) {
      if (!seq.stmt.has_value()) continue;

      const auto& stmt = ctx.DerefStmt(*seq.stmt);
      if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
        defs[var_decl_stmt->name].insert(block.id);
      } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
        defs[var_assign_stmt->name].insert(block.id);
      }
    }
  }
  return defs;
}

void InitPhiFunctions(const SyntaxContext& ctx, ControlFlowGraph& cfg) {
  const std::vector<BlockRef> idoms = ComputeImmediateDominators(cfg);
  const std::unordered_map<BlockRef, std::unordered_set<BlockRef>> dom_fronts =
      ComputeDominanceFrontiers(cfg, idoms);
  const std::unordered_map<std::string_view, std::unordered_set<BlockRef>>
      var_defs = CollectVarDefs(ctx, cfg);

  for (const auto& [var, def_blocks] : var_defs) {
    std::unordered_set<BlockRef> visited;
    std::unordered_set<BlockRef> pending = def_blocks;

    while (!pending.empty()) {
      auto block = *pending.begin();
      pending.erase(block);

      auto dom_front_it = dom_fronts.find(block);
      if (dom_front_it == dom_fronts.end()) continue;

      for (auto y : dom_front_it->second) {
        if (visited.contains(y)) continue;

        cfg.get(y).phis.push_back(std::string(var));
        visited.insert(y);

        if (!def_blocks.contains(y)) pending.insert(y);
      }
    }
  }
}

void RenameVariables(SyntaxContext& ctx, ControlFlowGraph& cfg) {
  int counter = 0;

  std::vector<std::unordered_map<std::string_view, std::string>> block_defs(
      cfg.blocks().Size());
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
    for (auto pred : block.preds) reaching_defs.merge(block_defs[pred]);

    for (size_t i = 0; i < block.phis.size(); ++i) {
      std::string new_name = block.phis[i] + std::to_string(counter++);
      reaching_defs[block.phis[i]] = new_name;
      block.phis[i] = new_name;
    }

    for (auto& seq : block.sequences) {
      for (size_t i = 0; i < seq.expressions.size(); ++i) {
        Expr& expr = ctx.DerefExpr(seq.expressions[i]);
        auto* ident_expr = std::get_if<IdentExpr>(&expr);
        if (ident_expr == nullptr) continue;

        ident_expr->name = reaching_defs[ident_expr->name];
      }

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
        var_assign_stmt->name = new_name;
      }
    }

    for (int i = block.next.size() - 1; i >= 0; --i) {
      if (!visited[block.next[i]]) pending.push(block.next[i]);
    }
  }
}

}  // namespace

void ConvertToStaticSingleAssignment(SyntaxContext& ctx,
                                     ControlFlowGraph& cfg) {
  InitPhiFunctions(ctx, cfg);
  RenameVariables(ctx, cfg);
}

}  // namespace lucid
