#include "lucid/am/reg.h"

#include <unistd.h>

#include <algorithm>
#include <bit>
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
#include "lucid/am/reg_set.h"
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
    const RegSet& live, const HashMap<Reg, std::size_t>& next_read,
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
  // One set for the whole walk: building one per block would cost a slot
  // for every register in the function each time, which is far more than
  // any one block has live.
  RegSet live(am_cfg.next_free_reg_id);

  for (const auto& block : am_cfg.Blocks()) {
    if (!liveness[block.ref.id()].has_value()) continue;

    // The walk backwards over the block starts from what is live where it
    // exits, and carries what is live at each instruction with it.
    live.Clear();
    for (Reg reg : LiveOut(am_cfg, liveness, block)) live.Insert(reg);

    // Returns the register to spill at the point the walk has reached, which
    // is ahead of the instruction at `from`, the `index`th of the block.
    //
    // An over full point always has one, unless everything live there is
    // spilled already and the spilling bought no room. That happens to a
    // value live across a phi function, which spilling does not yet reach.
    // See TODO.md.
    //
    // Where each register is next read is worked out here, from that point
    // on, rather than kept up at every instruction the walk passes. The walk
    // stops at the first point over full, and many reach none at all.
    const auto reg_to_spill = [&](std::list<Instruction>::const_iterator from,
                                  std::size_t index) {
      HashMap<Reg, std::size_t> next_read;
      for (auto it = from; it != block.instructions.end(); ++it, ++index) {
        // The first read found is the next one, and `Insert` keeps it.
        ForEachSourceRegister(*it,
                              [&](Reg reg) { next_read.Insert(reg, index); });
      }
      return RegReadFurthestAhead(live, next_read, spilled).value();
    };

    // True where more is live at the point the walk has reached than there
    // are registers to hold it.
    const auto over_full = [&] {
      return live.size() > static_cast<std::size_t>(max_clique_size);
    };

    if (over_full()) {
      return reg_to_spill(block.instructions.end(), block.instructions.size());
    }

    std::size_t index = block.instructions.size();
    for (auto it = block.instructions.rbegin(); it != block.instructions.rend();
         ++it) {
      --index;

      AbstractMachineLivenessAnalysis::TransferLive(live, *it);

      if (over_full()) return reg_to_spill(std::prev(it.base()), index);
    }

    if (block.ref == am_cfg.first) {
      for (Reg param : am_cfg.params) live.Insert(param);

      if (over_full()) return reg_to_spill(block.instructions.begin(), 0);
    }
  }
  return std::nullopt;
}

// The registers a phi function ties `reg` to: its result and each of its
// arguments, and whatever those are tied to in turn.
//
// They are the names one value goes by, reaching a block by one way or
// another, so putting one of them away is putting all of them away.
HashSet<Reg> TiedRegisters(const AbstractMachineControlFlowGraph& am_cfg,
                           Reg reg) {
  HashMap<Reg, std::vector<Reg>> ties;
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& phi : block.phis) {
      for (Reg src : phi.srcs) {
        ties.Emplace(phi.dst).push_back(src);
        ties.Emplace(src).push_back(phi.dst);
      }
    }
  }

  HashSet<Reg> tied;
  std::vector<Reg> to_walk = {reg};
  tied.Insert(reg);
  while (!to_walk.empty()) {
    const Reg from = to_walk.back();
    to_walk.pop_back();

    auto next = ties.Get(from);
    if (!next.has_value()) continue;

    for (Reg to : *next) {
      if (tied.Insert(to)) to_walk.push_back(to);
    }
  }
  return tied;
}

// Takes the registers in `to_spill` out of the registers, putting each away
// where it is written and loading its own copy back for every read of it.
//
// They go in the one slot between them, because they are the names one value
// goes by where phi functions tie them together. Those phi functions are then
// nothing left to carry out: whichever way control reached the block, the
// slot holds what the value came to.
void SpillRegisters(const HashSet<Reg>& to_spill,
                    AbstractMachineControlFlowGraph& am_cfg) {
  const std::size_t slot = am_cfg.stack_slots.size();
  am_cfg.stack_slots.push_back(to_spill.begin()->size == RegSize32 ? 4 : 8);

  // Puts the register away after the instruction that wrote it, and leaves the
  // walk standing on the store, which the step the walk takes next carries it
  // past. Standing on the instruction before it would read the store as one
  // more use and load the register back to store it again.
  auto maybe_insert_store = [&](std::list<Instruction>& instructions,
                                std::list<Instruction>::iterator& pos,
                                Reg reg) {
    if (!to_spill.Contains(reg)) return;

    pos = instructions.insert(std::next(pos), StoreStack{
                                                  .offset = slot,
                                                  .src_reg = reg,
                                              });
  };
  // Reads the register back before the instruction that uses it, into a
  // register of that use's own, and leaves the walk where it is: an
  // instruction can use it more than once, and the one after it can too.
  auto maybe_insert_load = [&](std::list<Instruction>& instructions,
                               std::list<Instruction>::iterator& pos,
                               Reg& reg) {
    if (!to_spill.Contains(reg)) return;

    reg.id = am_cfg.next_free_reg_id++;
    instructions.insert(pos, LoadStack{
                                 .offset = slot,
                                 .dst_reg = reg,
                             });
  };

  std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs =
      Vertices(am_cfg);
  std::sort(block_refs.begin(), block_refs.end(),
            CompareReversePostOrder(am_cfg));

  for (const auto& block_ref : block_refs) {
    auto& block = am_cfg.GetBlock(block_ref);

    // A parameter is in its register from the entry, because that is where
    // the caller leaves it, so it is put away at the top of the block ahead
    // of everything the block does. The walk starts after the store, so that
    // it does not read it as one more use of what it stores.
    auto i = block.instructions.begin();
    if (block_ref == am_cfg.first) {
      for (Reg param : am_cfg.params) {
        if (!to_spill.Contains(param)) continue;

        block.instructions.insert(i, StoreStack{
                                         .offset = slot,
                                         .src_reg = param,
                                     });
      }
    }
    while (i != block.instructions.end()) {
      ForEachSourceRegister(
          *i, [&](Reg& reg) { maybe_insert_load(block.instructions, i, reg); });
      if (Reg* target = TargetRegister(*i)) {
        maybe_insert_store(block.instructions, i, *target);
      }

      ++i;
    }
    if (block.branch_cond.has_value()) {
      maybe_insert_load(block.instructions, i, *block.branch_cond);
    }
  }

  // A phi function whose result is put away has every argument put away
  // beside it, in the same slot, so there is nothing left for it to settle.
  // Each argument was stored where its own block wrote it, and each read of
  // the result loads from that same slot, so the value is where it needs to
  // be by whichever way control came.
  //
  // This is what buys room where the sides of a branch meet. A phi function
  // held its arguments in registers to the end of every block they came
  // from, which no amount of spilling could take back so long as the phi was
  // there to read them.
  for (const auto& block_ref : block_refs) {
    auto& block = am_cfg.GetBlock(block_ref);
    std::erase_if(block.phis,
                  [&](const auto& phi) { return to_spill.Contains(phi.dst); });
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
void RemoveSpiltRegisters(const AbstractMachineControlFlowGraph& am_cfg,
                          const HashSet<Reg>& spilt,
                          AbstractMachineLiveness& liveness) {
  for (Reg reg : spilt) {
    const bool is_param =
        std::ranges::find(am_cfg.params, reg) != am_cfg.params.end();

    for (std::size_t id = 0; id < liveness.size(); ++id) {
      if (!liveness[id].has_value()) continue;
      if (is_param && std::size_t(am_cfg.first.id()) == id) continue;

      liveness[id]->live_in.Remove(reg);
    }
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

    // Whatever phi functions tie it to goes with it, because one value under
    // several names is put away once and read back by any of them.
    const HashSet<Reg> tied = TiedRegisters(am_cfg, *reg_to_spill);
    for (Reg reg : tied) spilt_regs.Insert(reg);

    SpillRegisters(tied, am_cfg);

    RemoveSpiltRegisters(am_cfg, tied, liveness);
    assert(MatchesFreshAnalysis(am_cfg, liveness));
  }
}

HashMap<Reg, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const InterferenceGraph& am_ig, int colors_count) {
  HashMap<Reg, int> reg_scores;
  for (Reg param : am_cfg.params) {
    reg_scores.Insert(param, 0);
  }
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& inst : block.instructions) {
      if (const Reg* target = TargetRegister(inst)) {
        reg_scores.Insert(*target, 0);
      }
      ForEachSourceRegister(inst, [&](Reg reg) { reg_scores.Insert(reg, 0); });
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

  // What each register scores, where it sits in its bucket, and whether it
  // has been taken, all held by register id rather than in a table keyed by
  // the register: the ids run from zero without gaps, so an index reaches
  // them, and each of these is read once per edge of the graph.
  const std::size_t reg_ids = am_cfg.next_free_reg_id;
  std::vector<int> scores(reg_ids, -1);
  std::vector<std::size_t> slots(reg_ids, 0);
  std::vector<bool> taken(reg_ids, false);

  // A score counts neighbours already taken, so none can reach the number of
  // registers there are.
  std::vector<std::vector<Reg>> buckets(regs_count + 1);
  for (const auto& [reg, score] : reg_scores) {
    slots[reg.id] = buckets[0].size();
    scores[reg.id] = 0;
    buckets[0].push_back(reg);
  }

  // Moves `reg` from the bucket of `score` to the one above it. The register
  // that was last in the bucket takes its place, so neither move is a search.
  const auto raise = [&](Reg reg, int score) {
    std::vector<Reg>& bucket = buckets[score];
    const std::size_t slot = slots[reg.id];
    bucket[slot] = bucket.back();
    slots[bucket[slot].id] = slot;
    bucket.pop_back();

    slots[reg.id] = buckets[score + 1].size();
    buckets[score + 1].push_back(reg);
  };

  std::vector<Reg> seo;
  seo.reserve(regs_count);
  std::size_t top = 0;
  while (seo.size() < regs_count) {
    while (buckets[top].empty()) --top;

    const Reg max_reg = buckets[top].back();
    buckets[top].pop_back();

    seo.push_back(max_reg);
    taken[max_reg.id] = true;

    for (Reg nb : am_ig.Neighbours(max_reg)) {
      if (taken[nb.id]) continue;
      const int nb_score = scores[nb.id];
      if (nb_score < 0) continue;

      raise(nb, nb_score);
      scores[nb.id] = nb_score + 1;
      // Everything still waiting scored at most `top` before this, so a
      // register can only ever be raised to the bucket just above it.
      top = std::max<std::size_t>(top, static_cast<std::size_t>(nb_score) + 1);
    }
  }

  // Which colour each register took, by register id, and none to begin with.
  // A colour is read once per edge of the graph, which is what makes this
  // worth an index rather than a hash.
  static constexpr int kNoColor = -1;
  std::vector<int> colors_by_reg(reg_ids, kNoColor);

  for (Reg reg : seo) {
    // The colours still free, a bit each, lowest first. There are no more of
    // them than there are registers to give, so they all fit in one word.
    std::uint64_t free_colors = colors_count >= 64
                                    ? ~std::uint64_t{0}
                                    : (std::uint64_t{1} << colors_count) - 1;

    for (Reg nb : am_ig.Neighbours(reg)) {
      const int neighbour_color = colors_by_reg[nb.id];
      if (neighbour_color != kNoColor) {
        free_colors &= ~(std::uint64_t{1} << (neighbour_color - 19));
      }
    }

    // The lowest of the colours left, which is what the register takes.
    //
    // There being none left means this register interferes with one of every
    // colour, which is more live at once than there are registers to hold it
    // and something the spilling was meant to have seen to. Checked in every
    // build: the colours are what the code is written against, so taking one
    // that was not free is wrong code rather than a slower answer.
    const std::optional<int> color =
        free_colors == 0
            ? std::nullopt
            : std::optional<int>(19 + std::countr_zero(free_colors));
    colors_by_reg[reg.id] = color.value();
  }

  HashMap<Reg, int> ig_colors;
  for (Reg reg : seo) ig_colors.Insert(reg, colors_by_reg[reg.id]);
  return ig_colors;
}

void UpdateRegister(const HashMap<Reg, int>& reg_colors, Reg& reg) {
  std::optional<const int&> color = reg_colors.Get(reg);
  assert(color.has_value());
  reg.id = *color;
}

void MergeRegisters(const HashMap<Reg, int>& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg) {
  for (Reg& param : am_cfg.params) UpdateRegister(reg_colors, param);
  for (auto& block : am_cfg.Blocks()) {
    for (auto& inst : block.instructions) {
      ForEachSourceRegister(inst,
                            [&](Reg& reg) { UpdateRegister(reg_colors, reg); });
      if (Reg* target = TargetRegister(inst)) {
        UpdateRegister(reg_colors, *target);
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
