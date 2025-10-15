#include "lucid/ssa.h"

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

}  // namespace

void ConvertToStaticSingleAssignment(const SyntaxContext& ctx,
                                     ControlFlowGraph& cfg) {
  const std::vector<BlockRef> idoms = ComputeImmediateDominators(cfg);
  const std::unordered_map<BlockRef, std::unordered_set<BlockRef>> dom_fronts =
      ComputeDominanceFrontiers(cfg, idoms);
  const std::unordered_map<std::string_view, std::unordered_set<BlockRef>>
      defs = CollectVarDefs(ctx, cfg);

  for (const auto& [var, def_blocks] : defs) {
    std::unordered_set<BlockRef> visited;
    std::unordered_set<BlockRef> pending = def_blocks;

    while (!pending.empty()) {
      auto block = *pending.begin();
      pending.erase(block);

      auto dom_front_it = dom_fronts.find(block);
      if (dom_front_it == dom_fronts.end()) continue;

      for (auto y : dom_front_it->second) {
        if (visited.contains(y)) continue;

        cfg.get(y).phis.push_back(var);
        visited.insert(y);

        if (!def_blocks.contains(y)) pending.insert(y);
      }
    }
  }
}

}  // namespace lucid
