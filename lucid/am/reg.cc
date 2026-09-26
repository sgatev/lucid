#include "lucid/am/reg.h"

#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <list>
#include <optional>
#include <ranges>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/state.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {
namespace {

// How far ahead a register is read when nothing in the block reads it. It is
// read in a block further on, which is further off than anything here.
constexpr std::size_t kReadBeyondTheBlock =
    std::numeric_limits<std::size_t>::max();

// Returns the register to spill out of those live at a point, which is the
// one read furthest ahead.
//
// Spilling a register costs a load wherever it is read, so the one whose
// reading is furthest off is the one that buys the most room for the fewest
// loads. Taking whichever came first instead is taking one at random, since
// what comes first out of a set of them is only where the hashing put it.
std::optional<Reg> RegReadFurthestAhead(
    const HashSet<Reg>& live, const HashMap<Reg, std::size_t>& next_read,
    const HashSet<Reg>& spilled) {
  std::optional<Reg> furthest;
  std::size_t furthest_read = 0;

  for (Reg reg : live) {
    if (spilled.Contains(reg)) continue;

    const auto read = next_read.Get(reg);
    const std::size_t at = read.has_value() ? *read : kReadBeyondTheBlock;
    if (furthest.has_value() && at <= furthest_read) continue;

    furthest = reg;
    furthest_read = at;
  }
  return furthest;
}

std::optional<Reg> FindRegToSpill(const AbstractMachineControlFlowGraph& am_cfg,
                                  const AbstractMachineLiveness& liveness,
                                  HashSet<Reg>& spilled, int max_clique_size) {
  for (const auto& block : am_cfg.Blocks()) {
    if (!liveness[block.ref.id()].has_value()) continue;

    // The walk backwards over the block starts from what is live where it
    // exits, and carries what is live at each instruction with it.
    AbstractMachineLivenessAnalysis::State state;
    state.live_in = LiveOut(am_cfg, liveness, block);

    // Where each register is next read, as an instruction's place in the
    // block. The walk runs backwards, so the last reading it writes down for
    // a register is the first one after wherever it has reached.
    HashMap<Reg, std::size_t> next_read;

    // True where more is live at the point the walk has reached than there
    // are registers to hold it.
    const auto over_full = [&] {
      return state.live_in.size() > max_clique_size;
    };

    // Returns the register to spill at the point the walk has reached.
    //
    // An over full point always has one, unless everything live there is
    // spilled already and the spilling bought no room. That happens to a
    // value live across a phi function, which spilling does not yet reach.
    // See TODO.md.
    const auto reg_to_spill = [&] {
      return RegReadFurthestAhead(state.live_in, next_read, spilled).value();
    };

    if (over_full()) return reg_to_spill();

    std::size_t index = block.instructions.size();
    for (const auto& inst : block.instructions | std::views::reverse) {
      --index;

      AbstractMachineLivenessAnalysis::Transfer(state, inst);
      ForEachSourceRegister(inst, [&](Reg reg) { next_read.Set(reg, index); });

      if (over_full()) return reg_to_spill();
    }

    if (block.ref == am_cfg.first) {
      for (auto& param : am_cfg.params) state.live_in.Insert(param);

      if (over_full()) return reg_to_spill();
    }
  }
  return std::nullopt;
}

void SpillRegisters(Reg reg_to_spill, AbstractMachineControlFlowGraph& am_cfg,
                    AbstractMachineState& am_state, HashSet<Reg>& spilt) {
  HashMap<Reg, std::size_t> reg_stack;

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
      maybe_insert_store(block.instructions, i, phi.dst);
    }
    while (i != block.instructions.end()) {
      auto& inst = *i;

      if (auto* cinst = std::get_if<MoveReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->src_reg);
        maybe_insert_store(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetReg>(&inst)) {
        maybe_insert_store(block.instructions, i, cinst->dst_reg);
      } else if (auto* cinst = std::get_if<SetInt>(&inst)) {
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
      } else if (auto* cinst = std::get_if<GeReg>(&inst)) {
        maybe_insert_load32(block.instructions, i, cinst->lhs_reg);
        maybe_insert_load32(block.instructions, i, cinst->rhs_reg);
        maybe_insert_store(block.instructions, i, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LeReg>(&inst)) {
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

  // A phi reads each of its arguments where control leaves the block that
  // argument comes from, so a spilt one is loaded back at the end of that
  // block and the phi takes the register the load wrote. Every phi reading
  // the register out of the same block reads the one load.
  //
  // The loads go in once the walk above has been everywhere, because the
  // block an argument comes from can be one the walk reaches after the block
  // its phi is in, and a load needs the slot the store settled on.
  //
  // What a load writes counts as spilt itself. It is live where the block
  // ends, which is the crowded point a phi's argument makes, so the search
  // would otherwise pick it, spill it, and write down another register just
  // like it, without end.
  std::vector<std::optional<Reg>> loaded_leaving(am_cfg.Blocks().Size());
  for (const auto& block_ref : block_refs) {
    auto& block = am_cfg.GetBlock(block_ref);

    for (auto& phi : block.phis) {
      for (std::size_t arg = 0; arg < phi.srcs.size(); ++arg) {
        if (phi.srcs[arg] != reg_to_spill) continue;

        const auto pred_ref = block.preds[arg];
        auto& loaded = loaded_leaving[pred_ref.id()];
        if (!loaded.has_value()) {
          Reg reg = reg_to_spill;
          reg.id = am_cfg.next_free_reg_id++;

          am_cfg.GetBlock(pred_ref).instructions.push_back(LoadStack{
              .offset = reg_stack.Get(reg_to_spill).value(),
              .dst_reg = reg,
          });
          spilt.Insert(reg);
          loaded = reg;
        }
        phi.srcs[arg] = *loaded;
      }
    }
  }
}

// Takes a register the spilling has just dealt with out of what is live
// where, in place of working the whole answer out again.
//
// Nothing reads a spilt register any longer but the store that follows where
// it is written, which stands in that same block and right after it, so the
// register is live at the head of no block at all. The registers the loads
// write reach no block's head either: each is written and read inside a
// single block, and the one a phi function's argument is loaded into is
// written where its block ends. So a spill only ever takes a register out of
// this answer, and never puts one in.
//
// A spilt parameter is the exception. It is live where the function is
// entered, because that is where the caller leaves it and the store that
// puts it away reads it there.
void RemoveSpiltRegister(const AbstractMachineControlFlowGraph& am_cfg, Reg reg,
                         AbstractMachineLiveness& liveness) {
  const bool is_param =
      std::ranges::find(am_cfg.params, reg) != am_cfg.params.end();

  for (std::size_t id = 0; id < liveness.size(); ++id) {
    if (!liveness[id].has_value()) continue;
    if (is_param && std::size_t(am_cfg.first.id()) == id) continue;

    liveness[id]->live_in.Remove(reg);
  }
}

// Whether what the spilling has been keeping up to date says what a fresh
// analysis of the graph would say.
//
// Carrying the answer forward is sound only so long as a spill changes
// nothing about the graph beyond the register it took out, which is a
// property of the rewriting rather than of anything checked here. This is
// what holds the two to each other, and it runs only where assertions do.
bool MatchesFreshAnalysis(const AbstractMachineControlFlowGraph& am_cfg,
                          const AbstractMachineLiveness& liveness) {
  AbstractMachineLivenessAnalysis analysis(am_cfg);
  const AbstractMachineLiveness fresh = RunDataflow(Backward(am_cfg), analysis);
  if (fresh.size() != liveness.size()) return false;

  for (std::size_t id = 0; id < fresh.size(); ++id) {
    if (fresh[id].has_value() != liveness[id].has_value()) return false;
    if (!fresh[id].has_value()) continue;
    if (fresh[id]->live_in.size() != liveness[id]->live_in.size()) return false;

    for (Reg reg : fresh[id]->live_in) {
      if (!liveness[id]->live_in.Contains(reg)) return false;
    }
  }
  return true;
}

}  // namespace

AbstractMachineLiveness SpillRegisters(AbstractMachineControlFlowGraph& am_cfg,
                                       AbstractMachineState& am_state,
                                       int max_clique_size) {
  HashSet<Reg> spilt_regs;

  // Worked out once and then carried through the spilling, which takes each
  // register it spills out of it rather than leaving the whole thing to be
  // worked out again.
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  AbstractMachineLiveness liveness =
      RunDataflow(Backward(am_cfg), liveness_analysis);

  while (true) {
    std::optional<Reg> reg_to_spill =
        FindRegToSpill(am_cfg, liveness, spilt_regs, max_clique_size);
    // Nothing left to spill, so this is what the graph as it stands is live
    // over, and whoever asked for the spilling is handed it.
    if (!reg_to_spill.has_value()) return liveness;

    spilt_regs.Insert(*reg_to_spill);
    SpillRegisters(*reg_to_spill, am_cfg, am_state, spilt_regs);

    RemoveSpiltRegister(am_cfg, *reg_to_spill, liveness);
    assert(MatchesFreshAnalysis(am_cfg, liveness));
  }
}

HashMap<Reg, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const InterferenceGraph& am_ig, int colors_count) {
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
      } else if (auto* cinst = std::get_if<SetInt>(&inst)) {
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
      } else if (auto* cinst = std::get_if<GeReg>(&inst)) {
        reg_scores.Insert(cinst->res_reg, 0);
        reg_scores.Insert(cinst->lhs_reg, 0);
        reg_scores.Insert(cinst->rhs_reg, 0);
      } else if (auto* cinst = std::get_if<LeReg>(&inst)) {
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
    }
    for (const auto& phi : block.phis) {
      reg_scores.Insert(phi.dst, 0);
      for (const auto& source : phi.srcs) reg_scores.Insert(source, 0);
    }
    if (block.branch_cond.has_value()) {
      reg_scores.Insert(*block.branch_cond, 0);
    }
  }

  // The registers in the order they are coloured in: each one taken has as
  // many already-taken neighbours as any register still to come.
  //
  // A register waits in the bucket of its score rather than being looked for
  // among all of them. Taking one raises each of its neighbours by a single
  // bucket, so the bucket to take from rises only when a register moves up
  // and falls only as buckets empty. That is one move per edge and one step
  // per register over the whole ordering, where searching for the highest
  // score meant reading every register still in the running for every
  // register taken.
  const std::size_t regs_count = reg_scores.size();

  // A score counts neighbours already taken, so none can reach the number of
  // registers there are.
  std::vector<std::vector<Reg>> buckets(regs_count + 1);
  HashMap<Reg, std::size_t> reg_slots;
  for (const auto& [reg, score] : reg_scores) {
    reg_slots.Insert(reg, buckets[0].size());
    buckets[0].push_back(reg);
  }

  // Moves `reg` from the bucket of `score` to the one above it. The register
  // that was last in the bucket takes its place, so neither move is a search.
  const auto raise = [&](Reg reg, int score) {
    std::vector<Reg>& bucket = buckets[score];
    const std::size_t slot = *reg_slots.Get(reg);
    bucket[slot] = bucket.back();
    reg_slots.Set(bucket[slot], slot);
    bucket.pop_back();

    reg_slots.Set(reg, buckets[score + 1].size());
    buckets[score + 1].push_back(reg);
  };

  HashSet<Reg> visited;
  std::vector<Reg> seo;
  seo.reserve(regs_count);
  std::size_t top = 0;
  while (seo.size() < regs_count) {
    while (buckets[top].empty()) --top;

    const Reg max_reg = buckets[top].back();
    buckets[top].pop_back();

    seo.push_back(max_reg);
    visited.Insert(max_reg);

    for (Reg nb : am_ig.Neighbours(max_reg)) {
      if (visited.Contains(nb)) continue;
      if (auto nb_score = reg_scores.Get(nb); nb_score.has_value()) {
        raise(nb, *nb_score);
        reg_scores.Set(nb, *nb_score + 1);
        // Everything still waiting scored at most `top` before this, so a
        // register can only ever be raised to the bucket just above it.
        top = std::max<std::size_t>(top, *nb_score + 1);
      }
    }
  }

  HashMap<Reg, int> ig_colors;
  for (Reg reg : seo) {
    HashSet<int> colors;
    for (int i = 0; i < colors_count; ++i) colors.Insert(19 + i);

    for (Reg nb : am_ig.Neighbours(reg)) {
      const auto& neighbour_color = ig_colors.Get(nb);
      if (neighbour_color.has_value()) colors.Remove(*neighbour_color);
    }

    // The lowest of the colours left, which is what the register takes.
    //
    // There being none left means this register interferes with one of every
    // colour, which is more live at once than there are registers to hold it
    // and something the spilling was meant to have seen to. Checked in every
    // build: the colours are what the code is written against, so taking one
    // that was not free is wrong code rather than a slower answer.
    std::optional<int> min_color;
    for (int color : colors) {
      if (!min_color.has_value() || color < *min_color) min_color = color;
    }

    ig_colors.Insert(reg, min_color.value());
  }

  return ig_colors;
}

void UpdateRegister(const HashMap<Reg, int>& reg_colors, Reg& reg) {
  std::optional<const int&> color = reg_colors.Get(reg);
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
      } else if (auto* cinst = std::get_if<SetInt>(&inst)) {
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
      } else if (auto* cinst = std::get_if<GeReg>(&inst)) {
        UpdateRegister(reg_colors, cinst->lhs_reg);
        UpdateRegister(reg_colors, cinst->rhs_reg);
        UpdateRegister(reg_colors, cinst->res_reg);
      } else if (auto* cinst = std::get_if<LeReg>(&inst)) {
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
