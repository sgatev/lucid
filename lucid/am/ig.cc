#include "lucid/am/ig.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/reg_set.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {
InterferenceGraph BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg) {
  AbstractMachineLivenessAnalysis liveness_analysis(am_cfg);
  return BuildInterferenceGraph(
      am_cfg, RunDataflow(Backward(am_cfg), liveness_analysis));
}

InterferenceGraph BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const AbstractMachineLiveness& liveness_block_states) {
  // The graph is built against register ids rather than against registers
  // hashed into a map. An id is handed out once and in sequence, so it is an
  // index; and reaching a register through an index, unlike inserting one
  // into a map, moves nothing the graph already holds. That is what lets a
  // register's neighbours be held on to while another register is added.
  const std::size_t reg_count = am_cfg.next_free_reg_id;

  std::vector<Reg> regs(reg_count);
  std::vector<bool> in_graph(reg_count, false);

  // The edges as they are found, and how many of them each register is at the
  // near end of. Neither says where a register's neighbours will lie, which
  // is only known once the last edge is in: the two together settle that in a
  // pass at the end.
  std::vector<std::pair<Reg, Reg>> edges;
  std::vector<std::uint32_t> degrees(reg_count, 0);

  // A bit for every register against every other, saying whether the edge
  // between them has been written down. The same edge is found wherever both
  // of its ends are live, and this is what a repeat is recognised by.
  std::vector<std::uint64_t> known_edges((reg_count * reg_count + 63) / 64);

  const auto put_in_graph = [&](Reg reg) {
    assert(reg.id >= 0 && std::size_t(reg.id) < reg_count);

    if (in_graph[reg.id]) return;

    in_graph[reg.id] = true;
    regs[reg.id] = reg;
  };

  const auto add_edge = [&](Reg from, Reg to) {
    put_in_graph(from);

    const std::size_t edge = std::size_t(from.id) * reg_count + to.id;
    std::uint64_t& known = known_edges[edge / 64];
    const std::uint64_t bit = std::uint64_t(1) << (edge % 64);
    if ((known & bit) != 0) return;

    known |= bit;
    ++degrees[from.id];
    edges.emplace_back(from, to);
  };

  // Every parameter is written where the function is entered, so no two of
  // them can share a register, whether or not the body ever reads them. The
  // entry puts them in their registers one after another, and one sharing
  // with another that has not been put in yet would be written over.
  for (Reg from : am_cfg.params) {
    put_in_graph(from);
    for (Reg to : am_cfg.params) {
      if (to != from) add_edge(from, to);
    }
  }

  // One set for the whole walk, emptied between blocks, rather than a slot
  // for every register in the function built again for each of them.
  RegSet live(reg_count);

  for (const auto& block : am_cfg.Blocks()) {
    if (!liveness_block_states[block.ref.id()].has_value()) continue;

    // The walk backwards over the block starts from what is live where it
    // exits, and carries what is live at each instruction with it.
    live.Clear();
    for (Reg reg : LiveOut(am_cfg, liveness_block_states, block)) {
      live.Insert(reg);
    }

    // Reused across the instructions of the block rather than rebuilt for
    // each: at most a couple of registers enter the live set at a time.
    std::vector<Reg> entering;

    for (Reg from : live) {
      put_in_graph(from);
      for (Reg to : live) {
        if (to != from) add_edge(from, to);
      }
    }

    for (const auto& inst : block.instructions | std::views::reverse) {
      if (auto target_reg = GetTargetRegister(inst); target_reg.has_value()) {
        put_in_graph(*target_reg);
        for (Reg to : live) {
          if (to != *target_reg) {
            add_edge(*target_reg, to);
            add_edge(to, *target_reg);
          }
        }
      }

      // Which of the registers this instruction reads are not live already.
      // Those are the ones the live set grows by, and so the only ones the
      // clique over it is missing: every pair that was live before this
      // instruction was joined when the later one was reached.
      entering.clear();
      ForEachSourceRegister(inst, [&](Reg reg) {
        if (!live.Contains(reg)) entering.push_back(reg);
      });

      AbstractMachineLivenessAnalysis::TransferLive(live, inst);

      if (auto* cinst = std::get_if<ModReg>(&inst)) {
        add_edge(cinst->res_reg, cinst->lhs_reg);
        add_edge(cinst->res_reg, cinst->rhs_reg);

        add_edge(cinst->lhs_reg, cinst->res_reg);
        add_edge(cinst->rhs_reg, cinst->res_reg);
      }

      for (Reg from : entering) {
        put_in_graph(from);
        for (Reg to : live) {
          if (to == from) continue;

          add_edge(from, to);
          add_edge(to, from);
        }
      }
    }

    for (const auto& phi : block.phis) put_in_graph(phi.dst);
  }

  // The ids the graph was built against are dropped here: what it is read
  // through is the registers themselves.
  // Where each register's neighbours begin, which is after everything the
  // registers before it have.
  std::vector<std::uint32_t> starts(reg_count + 1, 0);
  for (std::size_t id = 0; id < reg_count; ++id) {
    starts[id + 1] = starts[id] + degrees[id];
  }

  std::vector<Reg> neighbours(edges.size());
  std::vector<std::uint32_t> next(starts.begin(), starts.end() - 1);
  for (const auto& [from, to] : edges) neighbours[next[from.id]++] = to;

  std::vector<Reg> graph_regs;
  for (std::size_t id = 0; id < reg_count; ++id) {
    if (in_graph[id]) graph_regs.push_back(regs[id]);
  }

  return InterferenceGraph(std::move(graph_regs), std::move(starts),
                           std::move(neighbours));
}

}  // namespace lucid
