#include "lucid/reg.h"

#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <optional>
#include <ranges>
#include <stack>
#include <vector>

#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/dataflow.h"
#include "lucid/dom.h"
#include "lucid/hash_map.h"
#include "lucid/hash_set.h"
#include "lucid/liveness.h"
#include "lucid/string_index.h"

namespace lucid {
namespace {

HashMap<std::uint32_t, HashSet<std::uint32_t>> BuildDominatorTree(
    const ControlFlowGraph& cfg) {
  HashMap<std::uint32_t, HashSet<std::uint32_t>> dom_tree;
  std::vector<std::optional<ControlFlowGraph::BlockRef>> idoms =
      ComputeImmediateDominators(cfg);
  for (int i = 0; i < idoms.size(); ++i) {
    if (!idoms[i].has_value()) continue;

    std::uint32_t from = idoms[i]->id();
    std::uint32_t to = i;

    dom_tree.Insert(from, {});
    dom_tree.Find(from)->Insert(to);
  }
  return dom_tree;
}

}  // namespace

HashMap<StringIndex::Ref, HashSet<StringIndex::Ref>> BuildInterferenceGraph(
    const SyntaxContext& ctx, const ControlFlowGraph& cfg) {
  ControlFlowGraphAnalysis<LivenessAnalysis> liveness_analysis(cfg, ctx);
  std::vector<std::optional<LivenessAnalysis::State>> liveness_block_states =
      RunBackwardDataflow(cfg, liveness_analysis);

  HashMap<StringIndex::Ref, HashSet<StringIndex::Ref>> interference_graph;
  for (int i = 0; i < cfg.blocks().Size(); ++i) {
    const auto& state = liveness_block_states[i];
    if (!state.has_value()) continue;

    auto vars = state->live_in;
    for (const auto& seq : cfg.get(i).sequences) {
      const auto& stmt_ref = seq.stmt;
      if (!stmt_ref.has_value()) continue;
      const auto& stmt = ctx.DerefStmt(*stmt_ref);

      auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt);
      if (var_decl_stmt == nullptr) continue;

      vars.Insert(var_decl_stmt->name);
    }

    for (StringIndex::Ref from : vars) {
      interference_graph.Insert(from, {});
      for (StringIndex::Ref to : vars) {
        if (from == to) continue;

        interference_graph.Find(from)->Insert(to);
      }
    }
  }
  return interference_graph;
}

HashMap<StringIndex::Ref, int> ColorInterferenceGraph(
    const SyntaxContext& ctx, const ControlFlowGraph& cfg,
    const HashMap<StringIndex::Ref, HashSet<StringIndex::Ref>>& ig,
    int colors_count) {
  HashMap<StringIndex::Ref, int> ig_colors;

  HashMap<std::uint32_t, HashSet<std::uint32_t>> dom_tree =
      BuildDominatorTree(cfg);

  std::stack<std::uint32_t> pending;
  std::vector<int> visited(cfg.blocks().Size(), 0);

  pending.push(cfg.first.id());
  visited[cfg.first.id()] = 1;

  while (!pending.empty()) {
    auto block = pending.top();
    pending.pop();

    if (visited[block] == 2) {
      HashSet<int> colors;
      for (int i = 0; i < colors_count; ++i) colors.Insert(i);

      for (const auto& seq : cfg.get(block).sequences | std::views::reverse) {
        const auto& stmt_ref = seq.stmt;
        if (!stmt_ref.has_value()) continue;
        const auto& stmt = ctx.DerefStmt(*stmt_ref);

        auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt);
        if (var_decl_stmt == nullptr) continue;

        if (const auto& neighbours = ig.Find(var_decl_stmt->name);
            neighbours.has_value()) {
          for (const auto& neighbour : *neighbours) {
            const auto& neighbour_color = ig_colors.Find(neighbour);
            if (neighbour_color.has_value()) colors.Remove(*neighbour_color);
          }
        }

        assert(colors.begin() != colors.end());
        ig_colors.Insert(var_decl_stmt->name, *colors.begin());
      }
    } else {
      pending.push(block);
      visited[block] = 2;

      if (dom_tree.Find(block).has_value()) {
        for (auto next_block : *dom_tree.Find(block)) {
          if (visited[next_block] != 0) continue;

          pending.push(next_block);
          visited[next_block] = 1;
        }
      }
    }
  }

  return ig_colors;
}

}  // namespace lucid
