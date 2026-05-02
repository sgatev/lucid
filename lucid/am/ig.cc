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
      liveness_block_states = RunDataflow(Backward(am_cfg), liveness_analysis);

  HashMap<RegId, HashSet<RegId>> am_ig;
  for (const auto& block : am_cfg.blocks()) {
    auto& maybe_state = liveness_block_states[block.ref.id()];
    if (!maybe_state.has_value()) continue;
    auto& state = *maybe_state;

    for (const auto& next_block_ref : block.next) {
      const auto& next_block = am_cfg.get(next_block_ref);
      for (const auto& phi : next_block.phis) {
        for (int i = 0; i < next_block.preds.size(); ++i) {
          if (next_block.preds[i] == block.ref) {
            state.live_out.Insert(phi.sources[i]);
            break;
          }
        }
      }
    }

    state.live_in = std::move(state.live_out);

    for (RegId from : state.live_in) {
      am_ig.Insert(from, {});
      for (RegId to : state.live_in) {
        if (to != from) am_ig.Get(from)->Insert(to);
      }
    }

    for (const auto& inst : block.instructions | std::views::reverse) {
      if (auto target_reg = GetTargetRegister(inst); target_reg.has_value()) {
        for (RegId to : state.live_in) {
          if (to != *target_reg) {
            am_ig.Insert(*target_reg, {});
            am_ig.Get(*target_reg)->Insert(to);
            am_ig.Insert(to, {});
            am_ig.Get(to)->Insert(*target_reg);
          }
        }
      }

      state = AbstractMachineLivenessAnalysis::Transfer(std::move(state), inst);

      if (auto* cinst = std::get_if<ModReg>(&inst)) {
        am_ig.Insert(cinst->res_reg, {});
        am_ig.Insert(cinst->lhs_reg, {});
        am_ig.Insert(cinst->rhs_reg, {});

        am_ig.Get(cinst->res_reg)->Insert(cinst->lhs_reg);
        am_ig.Get(cinst->lhs_reg)->Insert(cinst->res_reg);

        am_ig.Get(cinst->res_reg)->Insert(cinst->rhs_reg);
        am_ig.Get(cinst->rhs_reg)->Insert(cinst->res_reg);
      }

      for (RegId from : state.live_in) {
        am_ig.Insert(from, {});
        for (RegId to : state.live_in) {
          if (to != from) am_ig.Get(from)->Insert(to);
        }
      }
    }

    for (const auto& phi : block.phis) {
      for (auto source : phi.sources) {
        am_ig.Insert(phi.target, {});
        am_ig.Get(phi.target)->Insert(source);

        am_ig.Insert(source, {});
        am_ig.Get(source)->Insert(phi.target);
      }
    }
  }

  return am_ig;
}

}  // namespace lucid
