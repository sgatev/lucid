#include "lucid/am/reg.h"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <list>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
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

// Returns where each register is next read in `block`, counting from the
// instruction at `from`, which is the `index`th of the block.
HashMap<Reg, std::size_t> NextReads(
    const AbstractMachineControlFlowGraph::Block& block,
    std::list<Instruction>::const_iterator from, std::size_t index) {
  HashMap<Reg, std::size_t> next_read;
  for (auto it = from; it != block.instructions.end(); ++it, ++index) {
    // The first read found is the next one, and `Insert` keeps it.
    ForEachSourceRegister(*it, [&](Reg reg) { next_read.Insert(reg, index); });
  }
  return next_read;
}

// Returns the register to spill at the first point in `block` where more is
// live than `max_clique_size`, walking it backwards from where it exits.
//
// `live` is only somewhere to keep what is live as the walk goes, handed in
// so that one set serves every block.
std::optional<Reg> FindRegToSpillInBlock(
    const AbstractMachineControlFlowGraph& am_cfg,
    const AbstractMachineLiveness& liveness,
    const AbstractMachineControlFlowGraph::Block& block,
    const HashSet<Reg>& spilled, int max_clique_size, RegSet& live) {
  // The walk starts from what is live where the block exits, and what the
  // branch reads there, and carries what is live at each instruction with it.
  live.Clear();
  for (Reg reg : LiveOut(liveness, block)) live.Insert(reg);
  if (block.branch_cond.has_value()) live.Insert(*block.branch_cond);

  // Returns the register to spill at the point the walk has reached, which
  // is ahead of the instruction at `from`, the `index`th of the block.
  //
  // An over full point always has one, unless everything live there is
  // spilled already and the spilling bought no room. That happens to a value
  // live across a phi function, which spilling does not yet reach. See
  // TODO.md.
  //
  // Where each register is next read is worked out here, from that point on,
  // rather than kept up at every instruction the walk passes. The walk stops
  // at the first point over full, and many reach none at all.
  const auto reg_to_spill = [&](std::list<Instruction>::const_iterator from,
                                std::size_t index) {
    return RegReadFurthestAhead(live, NextReads(block, from, index), spilled)
        .value();
  };

  // True where more is live at the point the walk has reached than there are
  // registers to hold it.
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
  return std::nullopt;
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
    const auto& block =
        *(blocks.begin() + static_cast<std::ptrdiff_t>(first_block));
    if (!liveness[block.ref.id()].has_value()) continue;

    const std::optional<Reg> reg = FindRegToSpillInBlock(
        am_cfg, liveness, block, spilled, max_clique_size, live);
    if (reg.has_value()) return reg;
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
// this answer, and never puts one in. The same goes for what is live where a
// block exits, which is what is live at the heads of the blocks after it and
// what their phi functions read, and a phi function whose result is spilt
// goes along with its arguments.
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
      auto& state = liveness[id];
      if (!state.has_value()) continue;

      state->live_out.Remove(reg);
      if (is_param && std::size_t(am_cfg.first.id()) == id) continue;
      state->live_in.Remove(reg);
    }
  }
}

// Colours as a bit each, lowest first. There are no more of them than there
// are registers to give, so they all fit in one word.
using Colors = std::uint64_t;

// The colour each register has been given so far, by register id.
class Coloring {
 public:
  Coloring(std::size_t reg_ids, int colors_count)
      : colors_(reg_ids, RegisterColors::kNone),
        all_colors_(~Colors{0} >> static_cast<unsigned>(64 - colors_count)) {}

  // Returns the colour `reg` has been given, as a bit.
  Colors Bit(Reg reg) const {
    assert(colors_[reg.id] != RegisterColors::kNone);
    return Colors{1} << static_cast<unsigned>(colors_[reg.id] - kFirstColor);
  }

  // Gives `reg` the lowest colour not in `taken`, and takes it.
  //
  // There being none left means more is live at once than there are
  // registers to hold it, which the spilling was meant to have seen to.
  // Checked in every build: the colours are what the code is written
  // against, so taking one that was not free is wrong code rather than a
  // slower answer.
  void Take(Reg reg, Colors& taken) {
    const Colors free = all_colors_ & ~taken;
    const std::optional<int> color =
        free == 0 ? std::nullopt : std::optional<int>(std::countr_zero(free));
    colors_[reg.id] = kFirstColor + color.value();
    taken |= Bit(reg);
  }

  RegisterColors Finalize() && { return RegisterColors(std::move(colors_)); }

 private:
  // The colours are the registers from 19 on, which a call hands back as it
  // found them.
  static constexpr int kFirstColor = 19;

  std::vector<int> colors_;
  Colors all_colors_;
};

// Where in a block each register is read for the last time, and whether it
// is live where the block exits, by register id. It is filled for one block
// and emptied again after it, so the slots for every register are made once
// rather than once for each block.
class LastReads {
 public:
  explicit LastReads(std::size_t reg_ids)
      : last_read_(reg_ids, -1), live_out_(reg_ids, false) {}

  void Fill(const AbstractMachineControlFlowGraph& am_cfg,
            const AbstractMachineLiveness& liveness,
            const AbstractMachineControlFlowGraph::Block& block) {
    std::int32_t at = 0;
    for (const auto& inst : block.instructions) {
      ForEachSourceRegister(inst, [&](Reg reg) { Mark(reg).last_read = at; });
      ++at;
    }
    // The branch reads what it decides on after everything the block does.
    if (block.branch_cond.has_value()) Mark(*block.branch_cond).last_read = at;
    for (Reg reg : LiveOut(liveness, block)) {
      Mark(reg).live_out = true;
    }
  }

  void Clear() {
    for (Reg reg : marked_) {
      last_read_[reg.id] = -1;
      live_out_[reg.id] = false;
    }
    marked_.clear();
  }

  // Whether the block reads `reg` for the last time at `at`, and so frees
  // its colour there.
  bool DiesAt(Reg reg, std::int32_t at) const {
    return !live_out_[reg.id] && last_read_[reg.id] == at;
  }

  // Whether nothing reads `reg` after it is written in the block. A register
  // is read only after it is written, so any read the block holds of it
  // stands later.
  bool Unread(Reg reg) const {
    return !live_out_[reg.id] && last_read_[reg.id] < 0;
  }

 private:
  struct Slots {
    std::int32_t& last_read;
    std::vector<bool>::reference live_out;
  };

  Slots Mark(Reg reg) {
    marked_.push_back(reg);
    return {last_read_[reg.id], live_out_[reg.id]};
  }

  std::vector<std::int32_t> last_read_;
  std::vector<bool> live_out_;
  std::vector<Reg> marked_;
};

// Colours every register `block` writes, keeping clear of the colours of
// what is live into it, which its dominators have already given out.
void ColorBlock(const AbstractMachineControlFlowGraph& am_cfg,
                const AbstractMachineLiveness& liveness,
                const AbstractMachineControlFlowGraph::Block& block,
                const LastReads& reads, Coloring& coloring) {
  Colors taken = 0;
  // The parameters are written where the function is entered, one after
  // another, so each has a colour of its own whether or not anything reads
  // it, and one that nothing reads gives its colour back once they all have
  // theirs.
  if (block.ref == am_cfg.first) {
    for (Reg param : am_cfg.params) coloring.Take(param, taken);
    for (Reg param : am_cfg.params) {
      if (reads.Unread(param)) taken &= ~coloring.Bit(param);
    }
  }
  for (Reg reg : liveness[block.ref.id()].value().live_in)
    taken |= coloring.Bit(reg);

  // The phi functions settle on their results together where the block
  // starts, beside everything live into it.
  for (const auto& phi : block.phis) coloring.Take(phi.dst, taken);
  for (const auto& phi : block.phis) {
    if (reads.Unread(phi.dst)) taken &= ~coloring.Bit(phi.dst);
  }

  // A register an instruction reads for the last time frees its colour for
  // the one the instruction writes, as nothing reads it after.
  std::int32_t at = 0;
  for (const auto& inst : block.instructions) {
    ForEachSourceRegister(inst, [&](Reg reg) {
      if (reads.DiesAt(reg, at)) taken &= ~coloring.Bit(reg);
    });
    if (const Reg* target = TargetRegister(inst)) {
      coloring.Take(*target, taken);
      if (reads.Unread(*target)) taken &= ~coloring.Bit(*target);
    }
    ++at;
  }
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
    const HashSet<Reg> tied = TiedRegisters(ties.value(), *reg_to_spill);
    for (Reg reg : tied) spilt_regs.Insert(reg);

    SpillRegisters(tied, am_cfg, rpo, occurrences.value());

    RemoveSpiltRegisters(am_cfg, tied, liveness);
  }
}

RegisterColors ColorRegisters(const AbstractMachineControlFlowGraph& am_cfg,
                              const AbstractMachineLiveness& liveness,
                              int colors_count) {
  assert(CheckStrictSsa(am_cfg).has_value());
  assert(colors_count > 0 && colors_count <= 64);

  Coloring coloring(am_cfg.next_free_reg_id, colors_count);
  LastReads reads(am_cfg.next_free_reg_id);

  // The blocks in reverse post-order, which comes to every block after all
  // of those that dominate it. A register live where a block starts is
  // written in one of those, so it has its colour by the time the block is
  // reached, and the colours of what is live there are what the block has
  // to keep clear of.
  std::vector<AbstractMachineControlFlowGraph::BlockRef> order =
      Vertices(am_cfg);
  std::sort(order.begin(), order.end(), CompareReversePostOrder(am_cfg));
  for (const auto ref : order) {
    const auto& block = am_cfg.GetBlock(ref);

    reads.Fill(am_cfg, liveness, block);
    ColorBlock(am_cfg, liveness, block, reads, coloring);
    reads.Clear();
  }

  return std::move(coloring).Finalize();
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
    if (auto& cond = block.branch_cond; cond.has_value()) {
      UpdateRegister(reg_colors, *cond);
    }
    for (auto& phi : block.phis) {
      UpdateRegister(reg_colors, phi.dst);
      for (auto& source : phi.srcs) UpdateRegister(reg_colors, source);
    }
  }
}

}  // namespace lucid
