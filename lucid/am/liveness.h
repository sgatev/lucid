#pragma once

#include <optional>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// Liveness dataflow analysis over an abstract machine control-flow graph.
class AbstractMachineLivenessAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    // Registers that are live before entering the block modeled by this state.
    HashSet<Reg> live_in;

    // Registers that are live after exiting the block modeled by this state.
    HashSet<Reg> live_out;
  };

  static State Transfer(State&& state, const Instruction& inst);

  explicit AbstractMachineLivenessAnalysis(
      const AbstractMachineControlFlowGraph& am_cfg);

  State Transfer(std::optional<State>&& prior_state,
                 const AbstractMachineControlFlowGraph::BlockRef& block_ref);

  State Join(State&& left, const State& right);

 private:
  const AbstractMachineControlFlowGraph& am_cfg_;
};

}  // namespace lucid
