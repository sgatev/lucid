#include "lucid/am_liveness.h"

#include "lucid/am_cfg.h"
#include "lucid/ast.h"

namespace lucid {

using State = AbstractMachineLivenessAnalysis::State;

AbstractMachineLivenessAnalysis::AbstractMachineLivenessAnalysis(
    const SyntaxContext& ctx)
    : ctx_(ctx) {}

State AbstractMachineLivenessAnalysis::MakeInitial() { return {}; }

State AbstractMachineLivenessAnalysis::Transfer(
    State state, const AbstractMachineControlFlowGraph::BlockRef& block) {
  // TODO
  return state;
}

State AbstractMachineLivenessAnalysis::Join(State left, State right) {
  State state;
  for (StringIndex::Ref ref : left.live_in) state.live_out.Insert(ref);
  for (StringIndex::Ref ref : right.live_in) state.live_out.Insert(ref);
  state.live_in = state.live_out;
  return state;
}

}  // namespace lucid
