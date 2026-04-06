#include "lucid/am/ig.h"

#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {

HashMap<RegId, HashSet<RegId>> BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      liveness_block_states = RunBackwardDataflow(am_cfg, liveness_analysis);

  HashMap<RegId, HashSet<RegId>> am_ig;
  for (const auto& block : am_cfg.blocks()) {
    auto& maybe_state = liveness_block_states[block.ref.id()];
    if (!maybe_state.has_value()) continue;
    auto& state = *maybe_state;

    state.live_in = std::move(state.live_out);

    for (RegId from : state.live_in) {
      am_ig.Insert(from, {});
      for (RegId to : state.live_in) {
        if (to != from) am_ig.Find(from)->Insert(to);
      }
    }

    for (const auto& inst : block.instructions | std::views::reverse) {
      state = AbstractMachineLivenessAnalysis::Transfer(std::move(state), inst);

      if (auto* cinst = std::get_if<ModReg32>(&inst)) {
        am_ig.Insert(cinst->res_reg, {});
        am_ig.Insert(cinst->lhs_reg, {});
        am_ig.Insert(cinst->rhs_reg, {});

        am_ig.Find(cinst->res_reg)->Insert(cinst->lhs_reg);
        am_ig.Find(cinst->lhs_reg)->Insert(cinst->res_reg);

        am_ig.Find(cinst->res_reg)->Insert(cinst->rhs_reg);
        am_ig.Find(cinst->rhs_reg)->Insert(cinst->res_reg);
      } else if (auto* cinst = std::get_if<ModReg64>(&inst)) {
        am_ig.Insert(cinst->res_reg, {});
        am_ig.Insert(cinst->lhs_reg, {});
        am_ig.Insert(cinst->rhs_reg, {});

        am_ig.Find(cinst->res_reg)->Insert(cinst->lhs_reg);
        am_ig.Find(cinst->lhs_reg)->Insert(cinst->res_reg);

        am_ig.Find(cinst->res_reg)->Insert(cinst->rhs_reg);
        am_ig.Find(cinst->rhs_reg)->Insert(cinst->res_reg);
      }

      for (RegId from : state.live_in) {
        am_ig.Insert(from, {});
        for (RegId to : state.live_in) {
          if (to != from) am_ig.Find(from)->Insert(to);
        }
      }
    }
  }

  return am_ig;
}

}  // namespace lucid
