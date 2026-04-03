#include "lucid/am/reg.h"

#include <unistd.h>

#include <cassert>
#include <optional>
#include <ranges>
#include <stack>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/core/container/graph/dominator.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/container/optional_ref.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {
namespace {

HashSet<RegId> FindRegistersToSpill(
    const AbstractMachineControlFlowGraph& am_cfg) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      liveness_block_states = RunBackwardDataflow(am_cfg, liveness_analysis);

  HashSet<RegId> registers_to_spill;
  for (const auto& block : am_cfg.blocks()) {
    auto maybe_state = liveness_block_states[block.ref.id()];
    if (!maybe_state.has_value()) continue;
    auto state = *maybe_state;

    state.live_in = state.live_out;

    for (const auto& inst : block.instructions | std::views::reverse) {
      state = AbstractMachineLivenessAnalysis::Transfer(std::move(state), inst);

      HashSet<RegId> clique;
      for (RegId reg : state.live_in) {
        clique.Insert(reg);
      }

      if (auto* cinst = std::get_if<ModReg32>(&inst)) {
        clique.Insert(cinst->lhs_reg);
        clique.Insert(cinst->rhs_reg);
        clique.Insert(cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
        clique.Insert(cinst->lhs_reg);
        clique.Insert(cinst->rhs_reg);
        clique.Insert(cinst->res_reg);
      }

      for (RegId spilled_reg : registers_to_spill) {
        clique.Remove(spilled_reg);
      }

      while (clique.size() > 12) {
        RegId spilled_reg = *clique.begin();
        clique.Remove(spilled_reg);
        registers_to_spill.Insert(spilled_reg);
      }
    }
  }
  return registers_to_spill;
}

void SpillRegisters(const HashSet<RegId>& registers_to_spill,
                    AbstractMachineControlFlowGraph& am_cfg,
                    std::vector<std::size_t>& stack_slots) {
  HashMap<RegId, std::size_t> reg_stack;
  auto maybe_insert_store32 = [&](std::vector<Instruction>& instructions,
                                  int& pos, RegId reg) {
    if (!registers_to_spill.Contains(reg)) return;

    ++pos;
    instructions.insert(instructions.begin() + pos,
                        StoreStack32{
                            .offset = stack_slots.size(),
                            .src_reg = reg,
                        });
    reg_stack.Insert(reg, stack_slots.size());
    stack_slots.push_back(4);
  };
  auto maybe_insert_store64 = [&](std::vector<Instruction>& instructions,
                                  int& pos, RegId reg) {
    if (!registers_to_spill.Contains(reg)) return;

    ++pos;
    instructions.insert(instructions.begin() + pos,
                        StoreStack64{
                            .offset = stack_slots.size(),
                            .src_reg = reg,
                        });
    reg_stack.Insert(reg, stack_slots.size());
    stack_slots.push_back(8);
  };
  auto maybe_insert_load32 = [&](std::vector<Instruction>& instructions,
                                 int& pos, RegId reg) {
    if (!registers_to_spill.Contains(reg)) return;

    instructions.insert(instructions.begin() + pos,
                        LoadStack32{
                            .offset = *reg_stack.Find(reg),
                            .dst_reg = reg,
                        });
    ++pos;
  };
  auto maybe_insert_load64 = [&](std::vector<Instruction>& instructions,
                                 int& pos, RegId reg) {
    if (!registers_to_spill.Contains(reg)) return;

    instructions.insert(instructions.begin() + pos,
                        LoadStack64{
                            .offset = *reg_stack.Find(reg),
                            .dst_reg = reg,
                        });
    ++pos;
  };

  for (auto& block : am_cfg.blocks()) {
    int i = 0;
    while (i < block.instructions.size()) {
      auto& inst = block.instructions[i];

      if (auto* cinst = std::get_if<MoveReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->dst_reg);
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
      } else if (auto* cinst = std::get_if<MoveReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
        maybe_insert_load64(block.instructions, i, cinst->src_reg);
      } else if (auto* cinst = std::get_if<SetReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<AddReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<AddReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<SubReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<SubReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<MulReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<MulReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<DivReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<DivReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<ModReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
      } else if (auto* cinst = std::get_if<StoreStack32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
      } else if (auto* cinst = std::get_if<StoreStackReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
        maybe_insert_load32(block.instructions, i, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<StoreStack64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->src_reg);
      } else if (auto* cinst = std::get_if<StoreStackReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->src_reg);
        maybe_insert_load64(block.instructions, i, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<LoadStack32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStackReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->dst_reg);
        maybe_insert_load32(block.instructions, i, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<LoadStack64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
        maybe_insert_load64(block.instructions, i, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
        for (const auto& arg : cinst->args) {
          if (arg.bits == 32) {
            maybe_insert_load32(block.instructions, i, arg.reg);
          } else {
            maybe_insert_load64(block.instructions, i, arg.reg);
          }
        }
        if (cinst->res.bits == 32) {
          maybe_insert_store32(block.instructions, i, cinst->res.reg);
        } else {
          maybe_insert_store64(block.instructions, i, cinst->res.reg);
        }
      } else if (auto* cinst = std::get_if<CondJump>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->cond_reg);
      }

      ++i;
    }
  }
}

}  // namespace

void IntroduceSpilling(AbstractMachineControlFlowGraph& am_cfg,
                       std::vector<std::size_t>& stack_slots) {
  SpillRegisters(FindRegistersToSpill(am_cfg), am_cfg, stack_slots);
}

HashMap<RegId, HashSet<RegId>> BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      liveness_block_states = RunBackwardDataflow(am_cfg, liveness_analysis);

  HashMap<RegId, HashSet<RegId>> interference_graph;
  for (const auto& block : am_cfg.blocks()) {
    auto maybe_state = liveness_block_states[block.ref.id()];
    if (!maybe_state.has_value()) continue;
    auto state = *maybe_state;

    state.live_in = state.live_out;

    for (RegId from : state.live_in) {
      interference_graph.Insert(from, {});
      for (RegId to : state.live_in) {
        if (from == to) continue;

        interference_graph.Find(from)->Insert(to);
      }
    }
    for (const auto& inst : block.instructions | std::views::reverse) {
      state = AbstractMachineLivenessAnalysis::Transfer(std::move(state), inst);

      if (auto* cinst = std::get_if<ModReg32>(&inst)) {
        interference_graph.Insert(cinst->res_reg, {});
        interference_graph.Insert(cinst->lhs_reg, {});
        interference_graph.Insert(cinst->rhs_reg, {});

        interference_graph.Find(cinst->res_reg)->Insert(cinst->lhs_reg);
        interference_graph.Find(cinst->lhs_reg)->Insert(cinst->res_reg);

        interference_graph.Find(cinst->res_reg)->Insert(cinst->rhs_reg);
        interference_graph.Find(cinst->rhs_reg)->Insert(cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
        interference_graph.Insert(cinst->res_reg, {});
        interference_graph.Insert(cinst->lhs_reg, {});
        interference_graph.Insert(cinst->rhs_reg, {});

        interference_graph.Find(cinst->res_reg)->Insert(cinst->lhs_reg);
        interference_graph.Find(cinst->lhs_reg)->Insert(cinst->res_reg);

        interference_graph.Find(cinst->res_reg)->Insert(cinst->rhs_reg);
        interference_graph.Find(cinst->rhs_reg)->Insert(cinst->res_reg);
      }

      for (RegId from : state.live_in) {
        interference_graph.Insert(from, {});
        for (RegId to : state.live_in) {
          if (from == to) continue;

          interference_graph.Find(from)->Insert(to);
        }
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
        } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
          dst_reg = cinst->res.reg;
        }
        if (!dst_reg.has_value()) continue;

        HashSet<int> colors;
        for (int i = 0; i <= colors_count; ++i) colors.Insert(i);

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

      if (auto* cinst = std::get_if<CondJump>(&inst)) {
        UpdateRegister(reg_colors, cinst->cond_reg);
      } else if (auto* cinst = std::get_if<MoveReg32>(&inst)) {
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
      } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
        for (auto& arg : cinst->args) UpdateRegister(reg_colors, arg.reg);
        UpdateRegister(reg_colors, cinst->res.reg);
      }
    }
  }
}

}  // namespace lucid
