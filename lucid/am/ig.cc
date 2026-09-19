#include "lucid/am/ig.h"

#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {

HashMap<Reg, HashSet<Reg>> BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      liveness_block_states = RunDataflow(Backward(am_cfg), liveness_analysis);

  HashMap<Reg, HashSet<Reg>> am_ig;
  for (const auto& block : am_cfg.Blocks()) {
    auto& maybe_state = liveness_block_states[block.ref.id()];
    if (!maybe_state.has_value()) continue;
    auto& state = *maybe_state;

    for (const auto& succ_ref : block.succs) {
      const auto& succ_block = am_cfg.GetBlock(succ_ref);
      for (const auto& phi : succ_block.phis) {
        for (int i = 0; i < succ_block.preds.size(); ++i) {
          if (succ_block.preds[i] == block.ref) {
            state.live_out.Insert(phi.srcs[i]);
            break;
          }
        }
      }
    }

    state.live_in = std::move(state.live_out);

    // Reused across the instructions of the block rather than rebuilt for
    // each: at most a couple of registers enter the live set at a time.
    std::vector<Reg> entering;

    for (Reg from : state.live_in) {
      auto& from_nbs = am_ig.Emplace(from);
      for (Reg to : state.live_in) {
        if (to != from) from_nbs.Insert(to);
      }
    }

    for (const auto& inst : block.instructions | std::views::reverse) {
      if (auto target_reg = GetTargetRegister(inst); target_reg.has_value()) {
        for (Reg to : state.live_in) {
          if (to != *target_reg) {
            am_ig.Emplace(*target_reg).Insert(to);
            am_ig.Emplace(to).Insert(*target_reg);
          }
        }
      }

      // Which of the registers this instruction reads are not live already.
      // Those are the ones the live set grows by, and so the only ones the
      // clique over it is missing: every pair that was live before this
      // instruction was joined when the later one was reached.
      entering.clear();
      ForEachSourceRegister(inst, [&](Reg reg) {
        if (!state.live_in.Contains(reg)) entering.push_back(reg);
      });

      state = AbstractMachineLivenessAnalysis::Transfer(std::move(state), inst);

      if (auto* cinst = std::get_if<ModReg>(&inst)) {
        am_ig.Emplace(cinst->res_reg).Insert(cinst->lhs_reg);
        am_ig.Emplace(cinst->lhs_reg).Insert(cinst->res_reg);

        am_ig.Emplace(cinst->res_reg).Insert(cinst->rhs_reg);
        am_ig.Emplace(cinst->rhs_reg).Insert(cinst->res_reg);
      }

      for (Reg from : entering) {
        // The edges back first. Adding a register to the graph may move the
        // ones already in it, so nothing may be held across that.
        for (Reg to : state.live_in) {
          if (to != from) am_ig.Emplace(to).Insert(from);
        }

        auto& from_nbs = am_ig.Emplace(from);
        for (Reg to : state.live_in) {
          if (to != from) from_nbs.Insert(to);
        }
      }
    }

    for (const auto& phi : block.phis) {
      for (auto source : phi.srcs) {
        am_ig.Emplace(phi.dst).Insert(source);
        am_ig.Emplace(source).Insert(phi.dst);
      }
    }
  }

  return am_ig;
}

}  // namespace lucid
