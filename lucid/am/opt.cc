#include "lucid/am/opt.h"

#include <cstddef>
#include <list>
#include <optional>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/hash_map.h"

namespace lucid {
namespace {

void RemoveUnnecessaryInstructions(std::list<Instruction>& instructions) {
  for (auto& inst : instructions) {
    // Remove MoveReg where src and dst registrars are the same.
    auto* cinst = std::get_if<MoveReg>(&inst);
    if (cinst == nullptr) continue;
    if (cinst->src_reg != cinst->dst_reg) continue;
    inst = Nop{};
  }
}

// Records for every branching block whether anything but its branch reads the
// value it branches on.
//
// A register is counted where it is written and where it is read, over the
// whole function, in one pass. A condition written once and read once is read
// by the branch alone, because the branch is one of those reads.
//
// Asking this after the registers have been given their colours would take a
// liveness analysis instead: a colour is shared by values that have nothing to
// do with one another, so what is live says little about what is read.
void MarkConditionsOnlyReadByBranches(AbstractMachineControlFlowGraph& am_cfg) {
  HashMap<Reg, int> writes;
  HashMap<Reg, int> reads;
  const auto count = [](HashMap<Reg, int>& counts, Reg reg) {
    if (auto found = counts.Get(reg); found.has_value()) {
      counts.Set(reg, *found + 1);
    } else {
      counts.Insert(reg, 1);
    }
  };

  for (Reg param : am_cfg.params) count(writes, param);
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& inst : block.instructions) {
      if (auto target = GetTargetRegister(inst); target.has_value()) {
        count(writes, *target);
      }
      ForEachSourceRegister(inst, [&](Reg reg) { count(reads, reg); });
    }
    for (const auto& phi : block.phis) {
      count(writes, phi.dst);
      for (Reg src : phi.srcs) count(reads, src);
    }
    if (block.branch_cond.has_value()) count(reads, *block.branch_cond);
  }

  for (auto& block : am_cfg.Blocks()) {
    if (!block.branch_cond.has_value()) continue;

    const std::optional<const int&> written = writes.Get(*block.branch_cond);
    const std::optional<const int&> read = reads.Get(*block.branch_cond);
    block.only_branch_reads_cond =
        written.has_value() && *written == 1 && read.has_value() && *read == 1;
  }
}

}  // namespace

void OptimizeAbstractMachineInstructions(std::list<Instruction>& instructions) {
  RemoveUnnecessaryInstructions(instructions);
}

void OptimizeAbstractMachineFunction(AbstractMachineControlFlowGraph& am_cfg) {
  for (auto& block : am_cfg.Blocks()) {
    OptimizeAbstractMachineInstructions(block.instructions);
  }
  MarkConditionsOnlyReadByBranches(am_cfg);
}

}  // namespace lucid
