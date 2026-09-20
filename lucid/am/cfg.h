#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>
#include <span>
#include <vector>

#include "lucid/am/instructions.h"
#include "lucid/core/container/arena.h"

namespace lucid {

// A graph that represents the control flow of abstract machine instructions.
class AbstractMachineControlFlowGraph {
 public:
  struct Block;

  // A reference to a block in the control flow graph.
  using BlockRef = Arena<Block>::Ref;

  // A null block reference.
  static constexpr BlockRef kNullBlockRef = Arena<Block>::kNullRef;

  using vertex_type = BlockRef;

  // A phi function in the control flow graph.
  struct Phi {
    // Destination for the result of the phi function.
    Reg dst;

    // Arguments to the phi function.
    std::vector<Reg> srcs;
  };

  // A basic block in the control flow graph.
  struct Block {
    // Reference of the basic block.
    BlockRef ref;

    // Phi functions in the basic block.
    std::vector<Phi> phis;

    // Instructions in the basic block, in order of exectuion.
    std::list<Instruction> instructions;

    // Successors of the basic block.
    std::vector<BlockRef> succs;

    // Predecessors of the basic block.
    std::vector<BlockRef> preds;

    // Condition register that determines the block in `next` that follows
    // this block. If set and the value is non-zero, control flow proceeds to
    // the first block in `next`. If set and the value is zero, control flow
    // proceeds to the second block in `next`.
    std::optional<Reg> branch_cond;

    // Whether nothing but the branch of this block reads `branch_cond`.
    //
    // A value read nowhere else does not outlive the block, so a backend is
    // free to let the branch compute it rather than put it in a register.
    //
    // Recorded here because it is only knowable before the registers are
    // given their colours: a colour is shared by values that have nothing to
    // do with one another, so afterwards the question takes a liveness
    // analysis to answer, and answers it for fewer blocks.
    bool only_branch_reads_cond = false;
  };

  // Adds a new block to the control flow graph.
  Block& AddBlock() {
    auto ref = blocks_.Add(Block());
    Block& block = blocks_.Get(ref);
    block.ref = ref;
    return block;
  }

  // Returns the block in the control flow graph refererenced by `ref`.
  //
  // `ref` must not be `kNullBlockRef`.
  auto& GetBlock(this auto&& self, BlockRef ref) {
    return self.blocks_.Get(ref);
  }

  // Returns an arena with all blocks that were added to the graph.
  auto& Blocks(this auto&& self) { return self.blocks_; }

  // Adds an edge to the control flow graph.
  void AddEdge(BlockRef from, BlockRef to) {
    GetBlock(from).succs.push_back(to);
    GetBlock(to).preds.push_back(from);
  }

  BlockRef first = kNullBlockRef;
  BlockRef last = kNullBlockRef;

  // Parameters of the function.
  std::vector<Reg> params;

  // Abstract machine stack slots.
  std::vector<int> stack_slots;

  // Next free register ID.
  std::int32_t next_free_reg_id = 1;

 private:
  Arena<Block> blocks_;
};

inline std::size_t VertexCount(const AbstractMachineControlFlowGraph& amcfg) {
  return amcfg.Blocks().Size();
}

inline std::vector<AbstractMachineControlFlowGraph::BlockRef> Vertices(
    const AbstractMachineControlFlowGraph& amcfg) {
  std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs;
  for (const auto& block : amcfg.Blocks()) block_refs.push_back(block.ref);
  return block_refs;
}

inline AbstractMachineControlFlowGraph::BlockRef SourceVertex(
    const AbstractMachineControlFlowGraph& amcfg) {
  return amcfg.first;
}

inline AbstractMachineControlFlowGraph::BlockRef SinkVertex(
    const AbstractMachineControlFlowGraph& amcfg) {
  return amcfg.last;
}

inline std::span<const AbstractMachineControlFlowGraph::BlockRef> NextVertices(
    const AbstractMachineControlFlowGraph& amcfg,
    AbstractMachineControlFlowGraph::BlockRef block_ref) {
  return amcfg.GetBlock(block_ref.id()).succs;
}

inline std::span<const AbstractMachineControlFlowGraph::BlockRef> PrevVertices(
    const AbstractMachineControlFlowGraph& amcfg,
    AbstractMachineControlFlowGraph::BlockRef block_ref) {
  return amcfg.GetBlock(block_ref.id()).preds;
}

inline std::uint32_t VertexId(
    const AbstractMachineControlFlowGraph&,
    AbstractMachineControlFlowGraph::BlockRef block_ref) {
  return block_ref.id();
}

}  // namespace lucid
