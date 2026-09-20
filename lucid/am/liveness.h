#pragma once

#include <optional>
#include <vector>

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
  };

  static void Transfer(State& state, const Instruction& inst);

  explicit AbstractMachineLivenessAnalysis(
      const AbstractMachineControlFlowGraph& am_cfg);

  State Transfer(std::optional<State>&& prior_state,
                 const AbstractMachineControlFlowGraph::BlockRef& block_ref);

  void Join(State& left, const State& right);

 private:
  const AbstractMachineControlFlowGraph& am_cfg_;
};

// Returns the registers that are live where `block` exits, given the `states`
// that an analysis over `am_cfg` settled on.
HashSet<Reg> LiveOut(
    const AbstractMachineControlFlowGraph& am_cfg,
    const std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>&
        states,
    const AbstractMachineControlFlowGraph::Block& block);

}  // namespace lucid
