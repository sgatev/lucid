#pragma once

#include <optional>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/reg_set.h"

namespace lucid {

// Liveness dataflow analysis over an abstract machine control-flow graph.
class AbstractMachineLivenessAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    // Registers that are live before entering the block modeled by this state.
    RegBitSet live_in;
  };

  static void Transfer(State& state, const Instruction& inst);

  // Takes `inst` backwards over what is live: the register it writes stops
  // being live where it stands, and the registers it reads start.
  //
  // Written against anything that can be added to and taken from, because
  // what holds the live registers differs with what is being worked out
  // over them, while the step itself does not.
  template <typename Live>
  static void TransferLive(Live& live, const Instruction& inst) {
    if (auto reg = GetTargetRegister(inst); reg.has_value()) live.Remove(*reg);
    ForEachSourceRegister(inst, [&](Reg reg) { live.Insert(reg); });
  }

  explicit AbstractMachineLivenessAnalysis(
      const AbstractMachineControlFlowGraph& am_cfg);

  State Transfer(std::optional<State>&& prior_state,
                 const AbstractMachineControlFlowGraph::BlockRef& block_ref);

  void Join(State& left, const State& right);

 private:
  const AbstractMachineControlFlowGraph& am_cfg_;
};

// What an analysis settled on for every block of a graph, indexed by block ID.
// A block the analysis never reached has nothing here.
using AbstractMachineLiveness =
    std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>;

// Returns the registers that are live where `block` exits, given the `states`
// that an analysis over `am_cfg` settled on.
RegBitSet LiveOut(const AbstractMachineControlFlowGraph& am_cfg,
                  const AbstractMachineLiveness& states,
                  const AbstractMachineControlFlowGraph::Block& block);

}  // namespace lucid
