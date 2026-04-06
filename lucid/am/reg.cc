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

std::optional<RegId> FindRegToSpill(
    const AbstractMachineControlFlowGraph& am_cfg, HashSet<RegId>& spilled) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      liveness_block_states = RunBackwardDataflow(am_cfg, liveness_analysis);

  for (const auto& block : am_cfg.blocks()) {
    auto maybe_state = liveness_block_states[block.ref.id()];
    if (!maybe_state.has_value()) continue;
    auto state = *maybe_state;

    state.live_in = state.live_out;

    if (state.live_in.size() > 10) {
      for (const auto& reg : state.live_in) {
        if (reg < 1000 && !spilled.Contains(reg)) return reg;
      }
      assert(false);
    }

    for (const auto& inst : block.instructions | std::views::reverse) {
      state = AbstractMachineLivenessAnalysis::Transfer(std::move(state), inst);

      if (state.live_in.size() > 10) {
        for (const auto& reg : state.live_in) {
          if (reg < 1000 && !spilled.Contains(reg)) return reg;
        }
        assert(false);
      }
    }
    if (block.ref == am_cfg.first) {
      for (auto& param : am_cfg.params) state.live_in.Insert(param.reg);

      if (state.live_in.size() > 10) {
        for (const auto& reg : state.live_in) {
          if (reg < 1000 && !spilled.Contains(reg)) return reg;
        }
        assert(false);
      }
    }
  }
  return std::nullopt;
}

void SpillRegisters(RegId reg_to_spill, AbstractMachineControlFlowGraph& am_cfg,
                    std::vector<std::size_t>& stack_slots, RegId& next_reg) {
  HashMap<RegId, std::size_t> reg_stack;
  HashMap<RegId, RegId> reg_rename;

  auto maybe_insert_store32 = [&](std::vector<Instruction>& instructions,
                                  int& pos, RegId reg) {
    if (reg != reg_to_spill) return;

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
    if (reg != reg_to_spill) return;

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
                                 int& pos, RegId& reg) {
    if (reg != reg_to_spill) return;

    RegId old_reg = reg;
    reg = next_reg++;
    reg_rename.Insert(old_reg, reg);

    auto offset = reg_stack.Find(old_reg);
    if (!offset.has_value()) return;

    instructions.insert(instructions.begin() + pos, LoadStack32{
                                                        .offset = *offset,
                                                        .dst_reg = reg,
                                                    });
    ++pos;
  };
  auto maybe_insert_load64 = [&](std::vector<Instruction>& instructions,
                                 int& pos, RegId& reg) {
    if (reg != reg_to_spill) return;

    RegId old_reg = reg;
    reg = next_reg++;
    reg_rename.Insert(old_reg, reg);

    auto offset = reg_stack.Find(old_reg);
    if (!offset.has_value()) return;

    instructions.insert(instructions.begin() + pos, LoadStack64{
                                                        .offset = *offset,
                                                        .dst_reg = reg,
                                                    });
    ++pos;
  };

  std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs =
      Vertices(am_cfg);
  const CompareVertexOrder<AbstractMachineControlFlowGraph> compare(
      am_cfg, ComputeReversePostOrder(am_cfg));
  std::sort(block_refs.begin(), block_refs.end(), compare);

  for (const auto& block_ref : block_refs) {
    auto& block = am_cfg.get(block_ref);

    int i = 0;
    if (block_ref == am_cfg.first) {
      for (auto& param : am_cfg.params) {
        maybe_insert_store32(block.instructions, i, param.reg);
      }
    }
    while (i < block.instructions.size()) {
      auto& inst = block.instructions[i];

      if (auto* cinst = std::get_if<MoveReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
        maybe_insert_store32(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<MoveReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->src_reg);
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg32>(&inst)) {
        maybe_insert_store32(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<AddReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<AddReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<SubReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<SubReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<MulReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<MulReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<DivReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<DivReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store32(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load64(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store64(block.instructions, i, cinst->res_reg);
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
        maybe_insert_load32(block.instructions, i, cinst->offset_reg);
        maybe_insert_store32(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStack64>(&inst)) {
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->offset_reg);
        maybe_insert_store64(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<Return>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
        for (auto& arg : cinst->args) {
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

void SpillRegisters(AbstractMachineControlFlowGraph& am_cfg,
                    std::vector<std::size_t>& stack_slots) {
  HashSet<RegId> spilt_regs;
  RegId next_reg = 1000;
  while (true) {
    std::optional<RegId> reg_to_spill = FindRegToSpill(am_cfg, spilt_regs);
    if (!reg_to_spill.has_value()) break;

    SpillRegisters(*reg_to_spill, am_cfg, stack_slots, next_reg);

    spilt_regs.Insert(*reg_to_spill);
  }
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
    auto block_ref = pending.top();
    pending.pop();

    if (visited[block_ref.id()] == 2) {
      auto& block = am_cfg.get(block_ref);
      for (const auto& inst : block.instructions | std::views::reverse) {
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
        for (int i = 0; i < colors_count; ++i) colors.Insert(19 + i);

        if (const auto& neighbours = ig.Find(*dst_reg);
            neighbours.has_value()) {
          for (const auto& neighbour : *neighbours) {
            const auto& neighbour_color = ig_colors.Find(neighbour);
            if (neighbour_color.has_value()) colors.Remove(*neighbour_color);
          }
        }

        assert(colors.begin() != colors.end());
        int min_color = *colors.begin();
        for (auto color : colors) {
          if (color < min_color) min_color = color;
        }

        ig_colors.Insert(*dst_reg, min_color);
      }
      if (block_ref == am_cfg.first) {
        for (auto& param : am_cfg.params) {
          RegId dst_reg = param.reg;

          HashSet<int> colors;
          for (int i = 0; i < colors_count; ++i) colors.Insert(19 + i);

          if (const auto& neighbours = ig.Find(dst_reg);
              neighbours.has_value()) {
            for (const auto& neighbour : *neighbours) {
              const auto& neighbour_color = ig_colors.Find(neighbour);
              if (neighbour_color.has_value()) colors.Remove(*neighbour_color);
            }
          }

          assert(colors.begin() != colors.end());
          int min_color = *colors.begin();
          for (auto color : colors) {
            if (color < min_color) min_color = color;
          }

          ig_colors.Insert(dst_reg, min_color);
        }
      }
    } else {
      pending.push(block_ref);
      visited[block_ref.id()] = 2;

      if (dom_tree.Find(block_ref).has_value()) {
        for (auto next_block : *dom_tree.Find(block_ref)) {
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
  for (auto& param : am_cfg.params) {
    UpdateRegister(reg_colors, param.reg);
  }
  for (auto& block : am_cfg.blocks()) {
    for (auto& inst : block.instructions) {
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
      } else if (auto* cinst = std::get_if<Return>(&inst)) {
        UpdateRegister(reg_colors, cinst->res_reg);
      }
    }
  }
}

}  // namespace lucid
