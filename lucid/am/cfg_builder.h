#pragma once

#include "lucid/am/cfg.h"

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
    am_cfg_.GetBlock(ref).instructions.push_back(std::move(inst));
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
  AbstractMachineControlFlowGraph am_cfg_;
};

}  // namespace lucid
