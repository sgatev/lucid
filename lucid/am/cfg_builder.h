#pragma once

#include <algorithm>
#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {

// Builds an abstract machine control flow graph.
class AbstractMachineControlFlowGraphBuilder {
 public:
  // Adds a block to the control flow graph.
  AbstractMachineControlFlowGraph::BlockRef AddBlock() {
    return am_cfg_.AddBlock().ref;
  }

  // Adds an edge between blocks in the control flow graph.
  void AddEdge(AbstractMachineControlFlowGraph::BlockRef from,
               AbstractMachineControlFlowGraph::BlockRef to) {
    am_cfg_.AddEdge(from, to);
  }

  // Adds an instruction to the control flow graph block.
  void AddInstruction(AbstractMachineControlFlowGraph::BlockRef ref,
                      Instruction inst) {
    if (auto target_reg = GetTargetRegister(inst); target_reg.has_value()) {
      TakeRegisterId(*target_reg);
    }
    ForEachSourceRegister(inst, [this](Reg reg) { TakeRegisterId(reg); });

    am_cfg_.GetBlock(ref).instructions.push_back(std::move(inst));
  }

  // Adds a phi function to the control flow graph block.
  void AddPhi(AbstractMachineControlFlowGraph::BlockRef ref,
              AbstractMachineControlFlowGraph::Phi phi) {
    TakeRegisterId(phi.dst);
    for (Reg source : phi.srcs) TakeRegisterId(source);

    am_cfg_.GetBlock(ref).phis.push_back(std::move(phi));
  }

  // Marks the first block of the control flow graph.
  void SetFirst(AbstractMachineControlFlowGraph::BlockRef ref) {
    am_cfg_.first = ref;
  }

  // Marks the last block of the control flow graph.
  void SetLast(AbstractMachineControlFlowGraph::BlockRef ref) {
    am_cfg_.last = ref;
  }

  // Returns the assembled control flow graph.
  AbstractMachineControlFlowGraph Build() && { return std::move(am_cfg_); }

 private:
  // Moves the next free register ID past `reg`, which a graph the translator
  // builds does as it hands the IDs out.
  void TakeRegisterId(Reg reg) {
    am_cfg_.next_free_reg_id = std::max(am_cfg_.next_free_reg_id, reg.id + 1);
  }

  AbstractMachineControlFlowGraph am_cfg_;
};

}  // namespace lucid
