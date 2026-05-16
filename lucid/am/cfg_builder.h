#pragma once

#include "lucid/am/cfg.h"

namespace lucid {

// Builds an abstract machine control flow graph.
class AbstractMachineControlFlowGraphBuilder {
 public:
  // Adds a block to the control flow graph.
  AbstractMachineControlFlowGraph::BlockRef block() {
    return am_cfg_.add().ref;
  }

  // Adds an edge between blocks in the control flow graph.
  void edge(AbstractMachineControlFlowGraph::BlockRef from,
            AbstractMachineControlFlowGraph::BlockRef to) {
    am_cfg_.get(from).next.push_back(to);
    am_cfg_.get(to).preds.push_back(from);
  }

  // Adds an instruction to the control flow graph block.
  void inst(AbstractMachineControlFlowGraph::BlockRef ref, Instruction inst) {
    am_cfg_.get(ref).instructions.push_back(std::move(inst));
  }

  // Marks the first block of the control flow graph.
  void first(AbstractMachineControlFlowGraph::BlockRef ref) {
    am_cfg_.first = ref;
  }

  // Marks the last block of the control flow graph.
  void last(AbstractMachineControlFlowGraph::BlockRef ref) {
    am_cfg_.last = ref;
  }

  // Returns the assembled control flow graph.
  AbstractMachineControlFlowGraph build() && { return std::move(am_cfg_); }

 private:
  AbstractMachineControlFlowGraph am_cfg_;
};

}  // namespace lucid
