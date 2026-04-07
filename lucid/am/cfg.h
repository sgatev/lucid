#pragma once

#include <cstdint>
#include <list>
#include <vector>

#include "lucid/am/instructions.h"
#include "lucid/core/container/arena.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// A graph that represents the control flow of a abstract machine instructions.
class AbstractMachineControlFlowGraph {
 public:
  struct Block;
  using BlockRef = Arena<Block>::Ref;
  using vertex_type = BlockRef;

  // A null block reference.
  static constexpr BlockRef kNullBlockRef = Arena<Block>::kNullRef;

  // Represents a basic block in the control flow graph.
  struct Block {
    BlockRef ref;
    std::list<Instruction> instructions;
    HashSet<BlockRef> next;
    HashSet<BlockRef> preds;
  };

  // Adds a new block to the control flow graph.
  Block& add() {
    auto ref = blocks_.Add(Block());
    Block& block = blocks_.Get(ref);
    block.ref = ref;
    return block;
  }

  // Returns the block in the control flow graph refererenced by `ref`.
  //
  // `ref` must not be `kNullBlockRef`.
  Block& get(BlockRef ref) { return blocks_.Get(ref); }

  // Returns the block in the control flow graph refererenced by `ref`.
  //
  // `ref` must not be `kNullBlockRef`.
  const Block& get(BlockRef ref) const { return blocks_.Get(ref); }

  // Returns an arena with all blocks that were added to the graph.
  Arena<Block>& blocks() { return blocks_; }
  const Arena<Block>& blocks() const { return blocks_; }

  BlockRef first = kNullBlockRef;
  BlockRef last = kNullBlockRef;

  // Represents a parameter of the function.
  struct Param {
    int bits;
    RegId reg;
  };

  // Parameters of the function.
  std::vector<Param> params;

 private:
  friend std::size_t VertexCount(const AbstractMachineControlFlowGraph&);
  friend std::vector<BlockRef> Vertices(const AbstractMachineControlFlowGraph&);
  friend std::vector<BlockRef> NextVertices(
      const AbstractMachineControlFlowGraph&, BlockRef);
  friend std::vector<BlockRef> PrevVertices(
      const AbstractMachineControlFlowGraph&, BlockRef);
  friend AbstractMachineControlFlowGraph::BlockRef SourceVertex(
      const AbstractMachineControlFlowGraph&);
  friend AbstractMachineControlFlowGraph::BlockRef SinkVertex(
      const AbstractMachineControlFlowGraph&);

  Arena<Block> blocks_;
};

inline std::size_t Hash(AbstractMachineControlFlowGraph::BlockRef v) {
  return Hash(v.id());
}

inline std::size_t VertexCount(const AbstractMachineControlFlowGraph& cfg) {
  return cfg.blocks_.Size();
}

inline std::vector<AbstractMachineControlFlowGraph::BlockRef> Vertices(
    const AbstractMachineControlFlowGraph& cfg) {
  std::vector<AbstractMachineControlFlowGraph::BlockRef> blocks;
  for (const auto& block : cfg.blocks_) blocks.push_back(block.ref);
  return blocks;
}

inline AbstractMachineControlFlowGraph::BlockRef SourceVertex(
    const AbstractMachineControlFlowGraph& cfg) {
  return cfg.first;
}

inline AbstractMachineControlFlowGraph::BlockRef SinkVertex(
    const AbstractMachineControlFlowGraph& cfg) {
  return cfg.last;
}

inline std::vector<AbstractMachineControlFlowGraph::BlockRef> NextVertices(
    const AbstractMachineControlFlowGraph& cfg,
    AbstractMachineControlFlowGraph::BlockRef block) {
  std::vector<AbstractMachineControlFlowGraph::BlockRef> next;
  for (const auto& block : cfg.get(block.id()).next) next.push_back(block);
  return next;
}

inline std::vector<AbstractMachineControlFlowGraph::BlockRef> PrevVertices(
    const AbstractMachineControlFlowGraph& cfg,
    AbstractMachineControlFlowGraph::BlockRef block) {
  std::vector<AbstractMachineControlFlowGraph::BlockRef> preds;
  for (const auto& block : cfg.get(block.id()).preds) preds.push_back(block);
  return preds;
}

inline std::uint32_t VertexId(const AbstractMachineControlFlowGraph&,
                              AbstractMachineControlFlowGraph::BlockRef block) {
  return block.id();
}

}  // namespace lucid
