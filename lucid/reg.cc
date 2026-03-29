#include "lucid/reg.h"

#include <unistd.h>

#include <cassert>
#include <optional>
#include <ranges>
#include <stack>
#include <variant>
#include <vector>

#include "lucid/am.h"
#include "lucid/am_cfg.h"
#include "lucid/am_liveness.h"
#include "lucid/core/container/graph/dominator.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/container/optional_ref.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {

HashMap<RegId, HashSet<RegId>> BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      liveness_block_states = RunBackwardDataflow(am_cfg, liveness_analysis);

  HashMap<RegId, HashSet<RegId>> interference_graph;
  for (auto block : Vertices(am_cfg)) {
    const auto& state = liveness_block_states[block.id()];
    if (!state.has_value()) continue;

    auto vars = state->live_in;
    for (RegId from : vars) {
      interference_graph.Insert(from, {});
      for (RegId to : vars) {
        if (from == to) continue;

        interference_graph.Find(from)->Insert(to);
      }
    }
  }
  return interference_graph;
}

HashMap<RegId, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const HashMap<RegId, HashSet<RegId>>& ig, int colors_count) {
  HashMap<RegId, int> ig_colors;

  std::vector<std::optional<AbstractMachineControlFlowGraph::BlockRef>> idoms =
      ComputeImmediateDominators(am_cfg);

  HashMap<AbstractMachineControlFlowGraph::BlockRef,
          HashSet<AbstractMachineControlFlowGraph::BlockRef>>
      dom_tree = BuildDominatorTree(am_cfg, idoms);

  std::stack<AbstractMachineControlFlowGraph::BlockRef> pending;
  std::vector<int> visited(VertexCount(am_cfg), 0);

  pending.push(SourceVertex(am_cfg));
  visited[SourceVertex(am_cfg).id()] = 1;

  while (!pending.empty()) {
    auto block = pending.top();
    pending.pop();

    if (visited[block.id()] == 2) {
      HashSet<int> colors;
      for (int i = 0; i < colors_count; ++i) colors.Insert(i);

      for (const auto& inst :
           am_cfg.get(block).instructions | std::views::reverse) {
        std::optional<RegId> dst_reg;
        if (auto* cinst = std::get_if<MoveReg32>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<MoveReg64>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<SetReg32>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<SetReg64>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<AddReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<AddReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<SubReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<SubReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<MulReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<MulReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<DivReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<DivReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<ModReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
          dst_reg = cinst->res_reg;
        } else if (auto* cinst = std::get_if<LoadStack32>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<LoadStackReg32>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<LoadStack64>(&inst)) {
          dst_reg = cinst->dst_reg;
        } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
          dst_reg = cinst->dst_reg;
        }
        if (!dst_reg.has_value()) continue;

        if (const auto& neighbours = ig.Find(*dst_reg);
            neighbours.has_value()) {
          for (const auto& neighbour : *neighbours) {
            const auto& neighbour_color = ig_colors.Find(neighbour);
            if (neighbour_color.has_value()) colors.Remove(*neighbour_color);
          }
        }

        assert(colors.begin() != colors.end());
        ig_colors.Insert(*dst_reg, *colors.begin());
      }
    } else {
      pending.push(block);
      visited[block.id()] = 2;

      if (dom_tree.Find(block).has_value()) {
        for (auto next_block : *dom_tree.Find(block)) {
          if (visited[next_block.id()] != 0) continue;

          pending.push(next_block);
          visited[next_block.id()] = 1;
        }
      }
    }
  }

  return ig_colors;
}

void UpdateRegister(const HashMap<RegId, int>& reg_colors, RegId& reg) {
  OptionalRef<int> color = reg_colors.Find(reg);
  assert(color.has_value());
  reg = *color;
}

void MergeRegisters(const HashMap<RegId, int>& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg) {
  for (auto& block : am_cfg.blocks()) {
    if (block.ref == am_cfg.last) continue;

    bool seen_label = false;
    for (auto& inst : block.instructions) {
      if (std::holds_alternative<Label>(inst)) {
        seen_label = true;
      }
      if (block.ref == am_cfg.first && !seen_label) continue;

      if (auto* cinst = std::get_if<MoveReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<MoveReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<AddReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<AddReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<SubReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<SubReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<MulReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<MulReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<DivReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<DivReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<StoreStack32>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
      } else if (auto* cinst = std::get_if<StoreStackReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
        UpdateRegister(reg_colors, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<StoreStack64>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
      } else if (auto* cinst = std::get_if<StoreStackReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
        UpdateRegister(reg_colors, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<LoadStack32>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStackReg32>(&inst)) {
        UpdateRegister(reg_colors, cinst->offset_reg);
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStack64>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
        UpdateRegister(reg_colors, cinst->offset_reg);
        UpdateRegister(reg_colors, cinst->dst_reg);
      }
    }
  }
}

}  // namespace lucid
