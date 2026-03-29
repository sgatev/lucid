#pragma once

#include "lucid/am.h"
#include "lucid/am_cfg.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// Liveness dataflow analysis over an abstract machine control-flow graph.
class AbstractMachineLivenessAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    // Registers that are live before entering the block modeled by this state.
    HashSet<RegId> live_in;

    // Registers that are live after exiting the block modeled by this state.
    HashSet<RegId> live_out;
  };

  static State Transfer(State state, Instruction inst);

  explicit AbstractMachineLivenessAnalysis(
      const AbstractMachineControlFlowGraph& am_cfg);

  State MakeInitial();

  State Transfer(State state,
                 const AbstractMachineControlFlowGraph::BlockRef& block);

  State Join(State left, State right);

 private:
  const AbstractMachineControlFlowGraph& am_cfg_;
};

}  // namespace lucid
