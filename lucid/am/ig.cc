#include "lucid/am/ig.h"

#include <cassert>
#include <cstddef>
#include <optional>
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

  // The graph is built against register ids rather than against registers
  // hashed into a map. An id is handed out once and in sequence, so it is an
  // index; and reaching a register through an index, unlike inserting one
  // into a map, moves nothing the graph already holds. That is what lets a
  // register's neighbours be held on to while another register is added.
  std::vector<std::optional<HashSet<Reg>>> neighbours(am_cfg.next_free_reg_id);
  std::vector<Reg> regs(am_cfg.next_free_reg_id);

  const auto nbs = [&](Reg reg) -> HashSet<Reg>& {
    assert(reg.id >= 0 && reg.id < am_cfg.next_free_reg_id);

    std::optional<HashSet<Reg>>& reg_nbs = neighbours[reg.id];
    if (!reg_nbs.has_value()) {
      reg_nbs.emplace();
      regs[reg.id] = reg;
    }
    return *reg_nbs;
  };

  for (const auto& block : am_cfg.Blocks()) {
    if (!liveness_block_states[block.ref.id()].has_value()) continue;

    // The walk backwards over the block starts from what is live where it
    // exits, and carries what is live at each instruction with it.
    AbstractMachineLivenessAnalysis::State state;
    state.live_in = LiveOut(am_cfg, liveness_block_states, block);

    // Reused across the instructions of the block rather than rebuilt for
    // each: at most a couple of registers enter the live set at a time.
    std::vector<Reg> entering;

    for (Reg from : state.live_in) {
      HashSet<Reg>& from_nbs = nbs(from);
      for (Reg to : state.live_in) {
        if (to != from) from_nbs.Insert(to);
      }
    }

    for (const auto& inst : block.instructions | std::views::reverse) {
      if (auto target_reg = GetTargetRegister(inst); target_reg.has_value()) {
        HashSet<Reg>& target_nbs = nbs(*target_reg);
        for (Reg to : state.live_in) {
          if (to != *target_reg) {
            target_nbs.Insert(to);
            nbs(to).Insert(*target_reg);
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

      AbstractMachineLivenessAnalysis::Transfer(state, inst);

      if (auto* cinst = std::get_if<ModReg>(&inst)) {
        HashSet<Reg>& res_nbs = nbs(cinst->res_reg);
        res_nbs.Insert(cinst->lhs_reg);
        res_nbs.Insert(cinst->rhs_reg);

        nbs(cinst->lhs_reg).Insert(cinst->res_reg);
        nbs(cinst->rhs_reg).Insert(cinst->res_reg);
      }

      for (Reg from : entering) {
        HashSet<Reg>& from_nbs = nbs(from);
        for (Reg to : state.live_in) {
          if (to == from) continue;

          from_nbs.Insert(to);
          nbs(to).Insert(from);
        }
      }
    }

    for (const auto& phi : block.phis) {
      HashSet<Reg>& dst_nbs = nbs(phi.dst);
      for (auto source : phi.srcs) {
        dst_nbs.Insert(source);
        nbs(source).Insert(phi.dst);
      }
    }
  }

  // The ids the graph was built against are dropped here: what it is read
  // through is the registers themselves.
  HashMap<Reg, HashSet<Reg>> am_ig;
  for (std::size_t id = 0; id < neighbours.size(); ++id) {
    if (!neighbours[id].has_value()) continue;

    am_ig.Insert(regs[id], *std::move(neighbours[id]));
  }

  return am_ig;
}

}  // namespace lucid
