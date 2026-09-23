#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"

namespace lucid {

// The registers of an abstract machine program that cannot share a colour,
// because they hold values that are live at the same time.
//
// The registers one interferes with lie end to end in a single run, and where
// each register's own stretch of that run begins is held beside it, indexed
// by register ID. Reaching a register costs an index rather than a hash, and
// reading what it interferes with is a walk straight through rather than a
// chase: what a colouring does with the graph is walk it, twice over.
class InterferenceGraph {
 public:
  InterferenceGraph() = default;

  InterferenceGraph(std::vector<Reg> regs, std::vector<std::uint32_t> starts,
                    std::vector<Reg> neighbours)
      : regs_(std::move(regs)),
        starts_(std::move(starts)),
        neighbours_(std::move(neighbours)) {}

  // Returns the registers the graph holds, by ascending ID.
  std::span<const Reg> Regs() const { return regs_; }

  // Returns the registers `reg` interferes with, which is none at all for a
  // register the graph does not hold.
  std::span<const Reg> Neighbours(Reg reg) const {
    if (reg.id < 0 || std::size_t(reg.id) + 1 >= starts_.size()) return {};

    const std::uint32_t start = starts_[reg.id];
    return std::span(neighbours_).subspan(start, starts_[reg.id + 1] - start);
  }

  // Returns the number of registers the graph holds.
  std::size_t size() const { return regs_.size(); }

  // Returns true iff the graph holds no registers.
  bool empty() const { return regs_.empty(); }

 private:
  std::vector<Reg> regs_;

  // Where the neighbours of the register with a given ID begin, and where the
  // ones of the ID after it do, which is where its own end. One longer than
  // there are IDs, so that the last has somewhere to end.
  std::vector<std::uint32_t> starts_;

  std::vector<Reg> neighbours_;
};

// Builds an interference graph of the abstract machine program given its
// control flow graph and what an analysis of it settled on.
//
// Working out what is live where is the greater part of the building, and
// spilling has already done it for the graph it leaves behind, so what it
// worked out is taken rather than worked out again.
InterferenceGraph BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const AbstractMachineLiveness& liveness);

// Builds an interference graph of the abstract machine program given its
// control flow graph, working out what is live where along the way.
InterferenceGraph BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
