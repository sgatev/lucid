#include "lucid/am/reg.h"

#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <list>
#include <optional>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/state.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/container/optional_ref.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {
namespace {

std::optional<Reg> FindRegToSpill(const AbstractMachineControlFlowGraph& am_cfg,
                                  HashSet<Reg>& spilled, int max_clique_size) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      liveness_block_states = RunDataflow(Backward(am_cfg), liveness_analysis);

  for (const auto& block : am_cfg.Blocks()) {
    auto maybe_state = liveness_block_states[block.ref.id()];
    if (!maybe_state.has_value()) continue;
    auto state = *maybe_state;

    for (int i = 0; i < block.next.size(); ++i) {
      const auto& next_block = am_cfg.GetBlock(block.next[i]);
      for (const auto& phi : next_block.phis) {
        state.live_out.Insert(phi.srcs[i]);
      }
    }

    state.live_in = state.live_out;

    if (state.live_in.size() > max_clique_size) {
      for (const auto& reg : state.live_in) {
        if (!spilled.Contains(reg)) return reg;
      }
      assert(false);
    }

    for (const auto& inst : block.instructions | std::views::reverse) {
      state = AbstractMachineLivenessAnalysis::Transfer(std::move(state), inst);

      if (state.live_in.size() > max_clique_size) {
        for (const auto& reg : state.live_in) {
          if (!spilled.Contains(reg)) return reg;
        }
        assert(false);
      }
    }
    if (block.ref == am_cfg.first) {
      for (auto& param : am_cfg.params) state.live_in.Insert(param);

      if (state.live_in.size() > max_clique_size) {
        for (const auto& reg : state.live_in) {
          if (!spilled.Contains(reg)) return reg;
        }
        assert(false);
      }
    }
  }
  return std::nullopt;
}

void SpillRegisters(Reg reg_to_spill, AbstractMachineControlFlowGraph& am_cfg,
                    AbstractMachineState& am_state) {
  HashMap<Reg, std::size_t> reg_stack;
  HashMap<Reg, Reg> reg_rename;

  auto maybe_insert_store = [&](std::list<Instruction>& instructions,
                                std::list<Instruction>::iterator& pos,
                                Reg reg) {
    if (reg != reg_to_spill) return;

    ++pos;
    reg_stack.Insert(reg, am_cfg.stack_slots.size());
    instructions.insert(pos, StoreStack{
                                 .offset = am_cfg.stack_slots.size(),
                                 .src_reg = reg,
                             });
    switch (reg.size) {
      case RegSize32:
        am_cfg.stack_slots.push_back(4);
        break;
      case RegSize64:
        am_cfg.stack_slots.push_back(8);
        break;
    }
  };
  auto maybe_insert_load32 = [&](std::list<Instruction>& instructions,
                                 std::list<Instruction>::iterator& pos,
                                 Reg& reg) {
    if (reg != reg_to_spill) return;

    Reg old_reg = reg;
    reg.id = am_cfg.next_free_reg_id++;
    reg_rename.Insert(old_reg, reg);

    auto offset = reg_stack.Get(old_reg);
    if (!offset.has_value()) return;

    instructions.insert(pos, LoadStack{
                                 .offset = *offset,
                                 .dst_reg = reg,
                             });
    ++pos;
  };
  auto maybe_insert_load64 = [&](std::list<Instruction>& instructions,
                                 std::list<Instruction>::iterator& pos,
                                 Reg& reg) {
    if (reg != reg_to_spill) return;

    Reg old_reg = reg;
    reg.id = am_cfg.next_free_reg_id++;
    reg_rename.Insert(old_reg, reg);

    auto offset = reg_stack.Get(old_reg);
    if (!offset.has_value()) return;

    instructions.insert(pos, LoadStack{
                                 .offset = *offset,
                                 .dst_reg = reg,
                             });
    ++pos;
  };

  std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs =
      Vertices(am_cfg);
  std::sort(block_refs.begin(), block_refs.end(),
            CompareReversePostOrder(am_cfg));

  for (const auto& block_ref : block_refs) {
    auto& block = am_cfg.GetBlock(block_ref);

    auto i = block.instructions.begin();
    if (block_ref == am_cfg.first) {
      for (auto& param : am_cfg.params) {
        maybe_insert_store(block.instructions, i, param);
      }
    }
    for (auto& phi : block.phis) {
      // TODO: Handle spilt phi sources.
      maybe_insert_store(block.instructions, i, phi.dst);
    }
    while (i != block.instructions.end()) {
      auto& inst = *i;

      if (auto* cinst = std::get_if<MoveReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
        maybe_insert_store(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg>(&inst)) {
        maybe_insert_store(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
        maybe_insert_store(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<AddReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<SubReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<MulReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<DivReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<GtReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LtReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<EqReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<NotEqReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<StoreStack>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
      } else if (auto* cinst = std::get_if<StoreStackReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
        maybe_insert_load32(block.instructions, i, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<LoadStack>(&inst)) {
        maybe_insert_store(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStackReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->offset_reg);
        maybe_insert_store(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<Return>(&inst)) {
        maybe_insert_load64(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
        for (auto& arg : cinst->args) {
          switch (arg.reg.size) {
            case RegSize32:
              maybe_insert_load32(block.instructions, i, arg.reg);
              break;
            case RegSize64:
              maybe_insert_load64(block.instructions, i, arg.reg);
              break;
          }
        }
        if (cinst->res.has_value()) {
          maybe_insert_store(block.instructions, i, cinst->res->reg);
        }
      }

      ++i;
    }
    if (block.branch_cond.has_value()) {
      maybe_insert_load32(block.instructions, i, *block.branch_cond);
    }
  }
}

}  // namespace

void SpillRegisters(AbstractMachineControlFlowGraph& am_cfg,
                    AbstractMachineState& am_state, int max_clique_size) {
  HashSet<Reg> spilt_regs;
  while (true) {
    std::optional<Reg> reg_to_spill =
        FindRegToSpill(am_cfg, spilt_regs, max_clique_size);
    if (!reg_to_spill.has_value()) break;

    SpillRegisters(*reg_to_spill, am_cfg, am_state);

    spilt_regs.Insert(*reg_to_spill);
  }
}

HashMap<Reg, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const HashMap<Reg, HashSet<Reg>>& am_ig, int colors_count) {
  HashMap<Reg, int> reg_scores;
  for (const auto& param : am_cfg.params) {
    reg_scores.Insert(param, 0);
  }
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& inst : block.instructions) {
      if (auto* cinst = std::get_if<MoveReg>(&inst)) {
        reg_scores.Insert(cinst->dst_reg, 0);
        reg_scores.Insert(cinst->src_reg, 0);
      } else if (auto* cinst = std::get_if<SetReg>(&inst)) {
        reg_scores.Insert(cinst->dst_reg, 0);
      } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
        reg_scores.Insert(cinst->dst_reg, 0);
      } else if (auto* cinst = std::get_if<AddReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<SubReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<MulReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<DivReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<ModReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<GtReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<LtReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<EqReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<NotEqReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<StoreStack>(&inst)) {
        reg_scores.Insert(cinst->src_reg, 0);
      } else if (auto* cinst = std::get_if<StoreStackReg>(&inst)) {
        reg_scores.Insert(cinst->src_reg, 0);
        reg_scores.Insert(cinst->offset_reg, 0);
      } else if (auto* cinst = std::get_if<LoadStack>(&inst)) {
        reg_scores.Insert(cinst->dst_reg, 0);
      } else if (auto* cinst = std::get_if<LoadStackReg>(&inst)) {
        reg_scores.Insert(cinst->dst_reg, 0);
        reg_scores.Insert(cinst->offset_reg, 0);
      } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
        if (cinst->res.has_value()) reg_scores.Insert(cinst->res->reg, 0);
        for (const auto& arg : cinst->args) reg_scores.Insert(arg.reg, 0);
      } else if (auto* cinst = std::get_if<Return>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
      } else {
        assert(false && "unhandled instruction type");
      }
      for (const auto& phi : block.phis) {
        reg_scores.Insert(phi.dst, 0);
        for (const auto& source : phi.srcs) reg_scores.Insert(source, 0);
      }
    }
    if (block.branch_cond.has_value()) {
      reg_scores.Insert(*block.branch_cond, 0);
    }
  }

  HashSet<Reg> visited;
  std::vector<Reg> seo;
  while (!reg_scores.empty()) {
    Reg max_reg = reg_scores.begin()->first;
    int max_score = reg_scores.begin()->second;
    for (const auto& [reg, score] : reg_scores) {
      if (score >= max_score) {
        max_reg = reg;
        max_score = score;
      }
    }

    seo.push_back(max_reg);
    reg_scores.Remove(max_reg);
    visited.Insert(max_reg);

    if (const auto& nbs = am_ig.Get(max_reg); nbs.has_value()) {
      for (const auto& nb : *nbs) {
        if (visited.Contains(nb)) continue;
        if (auto nb_score = reg_scores.Get(nb); nb_score.has_value()) {
          reg_scores.Set(nb, *nb_score + 1);
        }
      }
    }
  }

  HashMap<Reg, int> ig_colors;
  for (Reg reg : seo) {
    HashSet<int> colors;
    for (int i = 0; i < colors_count; ++i) colors.Insert(19 + i);

    if (const auto& nbs = am_ig.Get(reg); nbs.has_value()) {
      for (const auto& nb : *nbs) {
        const auto& neighbour_color = ig_colors.Get(nb);
        if (neighbour_color.has_value()) colors.Remove(*neighbour_color);
      }
    }

    assert(colors.begin() != colors.end());
    int min_color = *colors.begin();
    for (auto color : colors) {
      if (color < min_color) min_color = color;
    }

    ig_colors.Insert(reg, min_color);
  }

  return ig_colors;
}

void UpdateRegister(const HashMap<Reg, int>& reg_colors, Reg& reg) {
  OptionalRef<const int> color = reg_colors.Get(reg);
  assert(color.has_value());
  reg.id = *color;
}

void MergeRegisters(const HashMap<Reg, int>& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg) {
  for (auto& param : am_cfg.params) UpdateRegister(reg_colors, param);
  for (auto& block : am_cfg.Blocks()) {
    for (auto& inst : block.instructions) {
      if (auto* cinst = std::get_if<MoveReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<AddReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<SubReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<MulReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<DivReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<GtReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LtReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<EqReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<NotEqReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<StoreStack>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
      } else if (auto* cinst = std::get_if<StoreStackReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->src_reg);
        UpdateRegister(reg_colors, cinst->offset_reg);
      } else if (auto* cinst = std::get_if<LoadStack>(&inst)) {
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<LoadStackReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->offset_reg);
        UpdateRegister(reg_colors, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
        for (auto& arg : cinst->args) UpdateRegister(reg_colors, arg.reg);
        if (cinst->res.has_value()) UpdateRegister(reg_colors, cinst->res->reg);
      } else if (auto* cinst = std::get_if<Return>(&inst)) {
        UpdateRegister(reg_colors, cinst->res_reg);
      }
    }
    if (block.branch_cond.has_value()) {
      UpdateRegister(reg_colors, *block.branch_cond);
    }
    for (auto& phi : block.phis) {
      UpdateRegister(reg_colors, phi.dst);
      for (auto& source : phi.srcs) UpdateRegister(reg_colors, source);
    }
  }
}

}  // namespace lucid
