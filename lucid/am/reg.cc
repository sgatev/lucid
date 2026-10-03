#include "lucid/am/reg.h"

#include <unistd.h>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <list>
#include <optional>
#include <ranges>
#include <span>
#include <tuple>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/reg_set.h"
#include "lucid/am/state.h"
#include "lucid/am/strict_ssa.h"
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

// Returns the register to spill at the first point where more is live than
// `max_clique_size`, looking from the block at `first_block` among the
// blocks of `am_cfg` on, and leaves `first_block` at the block it was found
// in.
//
// The walk starts where the last one stopped rather than from the first
// block. A spill never leaves more live at a point than there was: a load's
// register stands in for the one put away, right before the read it serves,
// and a store follows a write the value was live after anyway. So every
// point before where the last walk stopped still has room, and a walk from
// the first block would come to the same block before it found anything.
std::optional<Reg> FindRegToSpill(const AbstractMachineControlFlowGraph& am_cfg,
                                  const AbstractMachineLiveness& liveness,
                                  HashSet<Reg>& spilled, int max_clique_size,
                                  std::size_t& first_block) {
  // One set for the whole walk: building one per block would cost a slot
  // for every register in the function each time, which is far more than
  // any one block has live.
  RegSet live(am_cfg.next_free_reg_id);

  const auto& blocks = am_cfg.Blocks();
  for (; first_block < blocks.Size(); ++first_block) {
    const auto& block = *(blocks.begin() + first_block);
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

// The registers each phi function ties together: its result to each of its
// arguments, both ways.
using PhiTies = HashMap<Reg, std::vector<Reg>>;

// Returns the ties the phi functions of `am_cfg` make.
//
// They are worked out once for all of the spilling. A phi function goes only
// when its result is spilt, and everything it ties is spilt with it, so the
// ties among the registers still to be spilt never change.
PhiTies TiesOf(const AbstractMachineControlFlowGraph& am_cfg) {
  PhiTies ties;
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& phi : block.phis) {
      for (Reg src : phi.srcs) {
        ties.Emplace(phi.dst).push_back(src);
        ties.Emplace(src).push_back(phi.dst);
      }
    }
  }
  return ties;
}

// The registers a phi function ties `reg` to: its result and each of its
// arguments, and whatever those are tied to in turn.
//
// They are the names one value goes by, reaching a block by one way or
// another, so putting one of them away is putting all of them away.
HashSet<Reg> TiedRegisters(const PhiTies& ties, Reg reg) {
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

// Where each register of a graph is named, kept up as the spilling rewrites
// the graph, so that putting a register away reaches the places that name it
// rather than every instruction there is, once for each register put away.
//
// A place is an instruction in a list, which stays where it is as others are
// put in around it, and the register in it, which is renamed in place.
class RegOccurrences {
 public:
  // What naming a register at a place asks of a spill there.
  enum class Kind : std::uint8_t {
    // A parameter, which the caller leaves in its register: stored where the
    // function is entered.
    kParam,
    // An instruction reads it: loaded right before.
    kRead,
    // An instruction writes it: stored right after.
    kWrite,
    // A block branches on it: loaded where the block ends.
    kBranch,
    // A phi function in a block gives it its value: nothing left to settle.
    kPhi,
  };

  struct Occurrence {
    Kind kind;
    // The block, by its place in reverse post-order, and the instruction, by
    // its place in the block as the graph was when this was built. A spill
    // goes through the places in this order, which is the order a walk of
    // the graph would come to them in.
    std::uint32_t block;
    std::uint32_t inst;
    // Which of the registers the instruction reads, in the order they are
    // read in.
    std::uint32_t operand;
    std::list<Instruction>::iterator at;
    Reg* reg;
  };

  // Where an instruction stands that comes after every other in its block,
  // and a register that is read after every other in its instruction.
  static constexpr std::uint32_t kLast =
      std::numeric_limits<std::uint32_t>::max();

  RegOccurrences(
      AbstractMachineControlFlowGraph& am_cfg,
      const std::vector<AbstractMachineControlFlowGraph::BlockRef>& rpo)
      : by_reg_(am_cfg.next_free_reg_id) {
    for (std::uint32_t b = 0; b < rpo.size(); ++b) {
      auto& block = am_cfg.GetBlock(rpo[b]);
      if (rpo[b] == am_cfg.first) {
        for (Reg& param : am_cfg.params) {
          Add({.kind = Kind::kParam, .block = b, .reg = &param});
        }
      }
      std::uint32_t inst = 1;
      for (auto it = block.instructions.begin(); it != block.instructions.end();
           ++it, ++inst) {
        std::uint32_t operand = 0;
        ForEachSourceRegister(*it, [&](Reg& reg) {
          Add({.kind = Kind::kRead,
               .block = b,
               .inst = inst,
               .operand = operand++,
               .at = it,
               .reg = &reg});
        });
        if (Reg* target = TargetRegister(*it)) {
          Add({.kind = Kind::kWrite,
               .block = b,
               .inst = inst,
               .operand = kLast,
               .at = it,
               .reg = target});
        }
      }
      if (block.branch_cond.has_value()) {
        Add({.kind = Kind::kBranch,
             .block = b,
             .inst = kLast,
             .reg = &*block.branch_cond});
      }
      for (auto& phi : block.phis) {
        Add({.kind = Kind::kPhi, .block = b, .reg = &phi.dst});
      }
    }
  }

  // Records that `occurrence` names the register it points at.
  void Add(const Occurrence& occurrence) {
    const std::size_t id = occurrence.reg->id;
    if (id >= by_reg_.size()) by_reg_.resize(id + 1);
    by_reg_[id].push_back(occurrence);
  }

  // Returns the places that name `reg`, in the order a walk comes to them.
  std::span<const Occurrence> Of(Reg reg) const { return by_reg_[reg.id]; }

 private:
  std::vector<std::vector<Occurrence>> by_reg_;
};

// Takes the registers in `to_spill` out of the registers, putting each away
// where it is written and loading its own copy back for every read of it.
//
// They go in the one slot between them, because they are the names one value
// goes by where phi functions tie them together. Those phi functions are then
// nothing left to carry out: whichever way control reached the block, the
// slot holds what the value came to.
//
// Only the places `occurrences` holds for them are visited, in the order a
// walk over the blocks in reverse post-order would come to them, so the
// loads take their registers in the order such a walk would give them out.
void SpillRegisters(
    const HashSet<Reg>& to_spill, AbstractMachineControlFlowGraph& am_cfg,
    const std::vector<AbstractMachineControlFlowGraph::BlockRef>& rpo,
    RegOccurrences& occurrences) {
  using Kind = RegOccurrences::Kind;
  using Occurrence = RegOccurrences::Occurrence;

  const std::size_t slot = am_cfg.stack_slots.size();
  am_cfg.stack_slots.push_back(to_spill.begin()->size == RegSize32 ? 4 : 8);

  std::vector<Occurrence> places;
  for (Reg reg : to_spill) {
    const auto of = occurrences.Of(reg);
    places.insert(places.end(), of.begin(), of.end());
  }
  // The places of one register are already in order. Those of several, which
  // phi functions tie together, are put in order between them.
  if (to_spill.size() > 1) {
    std::ranges::sort(places, {}, [](const Occurrence& place) {
      return std::tuple(place.block, place.inst, place.operand);
    });
  }

  // A parameter is put away ahead of everything the entry does, including
  // the loads earlier spills put there.
  auto& entry = am_cfg.GetBlock(am_cfg.first).instructions;
  const auto entry_begin = entry.begin();

  // Reads the register back into a register of its own, standing right
  // before `at`, and records where that register is named in turn: it can be
  // spilt as well, later on.
  const auto load = [&](const Occurrence& place, Kind kind,
                        std::list<Instruction>& instructions,
                        std::list<Instruction>::iterator at) {
    Reg& read = *place.reg;
    read.id = am_cfg.next_free_reg_id++;
    const auto loaded =
        instructions.insert(at, LoadStack{.offset = slot, .dst_reg = read});
    occurrences.Add({.kind = Kind::kWrite,
                     .block = place.block,
                     .inst = place.inst,
                     .at = loaded,
                     .reg = &std::get<LoadStack>(*loaded).dst_reg});
    Occurrence read_place = place;
    read_place.kind = kind;
    occurrences.Add(read_place);
  };

  for (const Occurrence& place : places) {
    auto& block = am_cfg.GetBlock(rpo[place.block]);
    switch (place.kind) {
      case Kind::kParam:
        entry.insert(entry_begin,
                     StoreStack{.offset = slot, .src_reg = *place.reg});
        break;
      case Kind::kRead:
        load(place, Kind::kRead, block.instructions, place.at);
        break;
      case Kind::kWrite:
        block.instructions.insert(
            std::next(place.at),
            StoreStack{.offset = slot, .src_reg = *place.reg});
        break;
      case Kind::kBranch:
        load(place, Kind::kBranch, block.instructions,
             block.instructions.end());
        break;
      case Kind::kPhi:
        // A phi function whose result is put away has every argument put
        // away beside it, in the same slot, so there is nothing left for it
        // to settle. Each argument was stored where its own block wrote it,
        // and each read of the result loads from that same slot, so the value
        // is where it needs to be by whichever way control came.
        //
        // This is what buys room where the sides of a branch meet. A phi
        // function held its arguments in registers to the end of every block
        // they came from, which no amount of spilling could take back so long
        // as the phi was there to read them.
        std::erase_if(block.phis, [&](const auto& phi) {
          return to_spill.Contains(phi.dst);
        });
        break;
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

  // What a spill needs to find its way around the graph, worked out at the
  // first one: most functions have room for everything and never spill.
  std::vector<AbstractMachineControlFlowGraph::BlockRef> rpo;
  std::optional<RegOccurrences> occurrences;
  std::optional<PhiTies> ties;

  std::size_t first_block = 0;
  while (true) {
    std::optional<Reg> reg_to_spill = FindRegToSpill(
        am_cfg, liveness, spilt_regs, max_clique_size, first_block);
    // Nothing left to spill, so this is what the graph as it stands is live
    // over, and whoever asked for the spilling is handed it.
    if (!reg_to_spill.has_value()) return liveness;

    if (!occurrences.has_value()) {
      rpo = Vertices(am_cfg);
      std::sort(rpo.begin(), rpo.end(), CompareReversePostOrder(am_cfg));
      occurrences.emplace(am_cfg, rpo);
      ties.emplace(TiesOf(am_cfg));
    }

    // Whatever phi functions tie it to goes with it, because one value under
    // several names is put away once and read back by any of them.
    const HashSet<Reg> tied = TiedRegisters(*ties, *reg_to_spill);
    for (Reg reg : tied) spilt_regs.Insert(reg);

    SpillRegisters(tied, am_cfg, rpo, *occurrences);

    RemoveSpiltRegisters(am_cfg, tied, liveness);
    assert(MatchesFreshAnalysis(am_cfg, liveness));
  }
}

RegisterColors ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const InterferenceGraph& am_ig, int colors_count) {
  assert(CheckStrictSsa(am_cfg).has_value());

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
  static constexpr int kNoColor = RegisterColors::kNone;
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

  return RegisterColors(std::move(colors_by_reg));
}

void UpdateRegister(const RegisterColors& reg_colors, Reg& reg) {
  const std::optional<int> color = reg_colors.Get(reg);
  assert(color.has_value());
  reg.id = *color;
}

void MergeRegisters(const RegisterColors& reg_colors,
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
