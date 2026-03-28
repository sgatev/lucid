#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include "lucid/am.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// A graph that represents the control flow of a abstract machine instructions.
class AbstractMachineControlFlowGraph {
 public:
  // Represents a reference to a basic block in the control flow graph.
  class BlockRef {
   public:
    explicit BlockRef(std::uint32_t id) : id_(id) {}

    // Returns the ID of the basic block.
    std::uint32_t id() const { return id_; }

    bool operator==(const BlockRef&) const = default;

   private:
    std::uint32_t id_;
  };

  // Represents a basic block in the control flow graph.
  class Block {
   public:
    explicit Block(std::uint32_t id) : id_(id) {}

    // Returns a reference to the basic block.
    BlockRef ref() const { return BlockRef(id_); }

    // Returns references to the basic blocks following this one in the control
    // flow graph.
    const HashSet<BlockRef>& NextBlocks() const { return next_blocks_; }

    // Returns references to the basic blocks preceding this one in the control
    // flow graph.
    const HashSet<BlockRef>& PreviousBlocks() const { return previous_blocks_; }

    // Returns the instructions of the basic block.
    std::span<const Instruction> Instructions() const { return instructions_; }

   private:
    friend AbstractMachineControlFlowGraph BuildAbstractMachineControlFlowGraph(
        const std::vector<Instruction>&);

    std::uint32_t id_;
    std::span<const Instruction> instructions_;
    HashSet<BlockRef> next_blocks_;
    HashSet<BlockRef> previous_blocks_;
  };

  using vertex_type = BlockRef;

  AbstractMachineControlFlowGraph(HashMap<std::uint32_t, Block> blocks)
      : blocks_(std::move(blocks)) {}

  OptionalRef<Block> Get(BlockRef ref) const { return blocks_.Find(ref.id()); }

 private:
  friend std::size_t VertexCount(const AbstractMachineControlFlowGraph&);
  friend std::vector<BlockRef> Vertices(const AbstractMachineControlFlowGraph&);
  friend std::vector<BlockRef> NextVertices(
      const AbstractMachineControlFlowGraph&, BlockRef);
  friend std::vector<BlockRef> PrevVertices(
      const AbstractMachineControlFlowGraph&, BlockRef);

  HashMap<std::uint32_t, Block> blocks_;
};

inline std::size_t Hash(AbstractMachineControlFlowGraph::BlockRef v) {
  return Hash(v.id());
}

inline std::size_t VertexCount(const AbstractMachineControlFlowGraph& cfg) {
  return cfg.blocks_.size();
}

inline std::vector<AbstractMachineControlFlowGraph::BlockRef> Vertices(
    const AbstractMachineControlFlowGraph& cfg) {
  std::vector<AbstractMachineControlFlowGraph::BlockRef> blocks;
  for (const auto& [_, block] : cfg.blocks_) {
    blocks.push_back(block.ref());
  }
  return blocks;
}

inline AbstractMachineControlFlowGraph::BlockRef SourceVertex(
    const AbstractMachineControlFlowGraph& cfg) {
  return AbstractMachineControlFlowGraph::BlockRef(0);
}

inline AbstractMachineControlFlowGraph::BlockRef SinkVertex(
    const AbstractMachineControlFlowGraph& cfg) {
  return AbstractMachineControlFlowGraph::BlockRef(1);
}

inline std::vector<AbstractMachineControlFlowGraph::BlockRef> NextVertices(
    const AbstractMachineControlFlowGraph& cfg,
    AbstractMachineControlFlowGraph::BlockRef block) {
  std::vector<AbstractMachineControlFlowGraph::BlockRef> next_blocks;
  if (auto v = cfg.blocks_.Find(block.id()); v.has_value()) {
    for (const auto& block : v->NextBlocks()) {
      next_blocks.push_back(block);
    }
  }
  return next_blocks;
}

inline std::vector<AbstractMachineControlFlowGraph::BlockRef> PrevVertices(
    const AbstractMachineControlFlowGraph& cfg,
    AbstractMachineControlFlowGraph::BlockRef block) {
  std::vector<AbstractMachineControlFlowGraph::BlockRef> previous_blocks;
  if (auto v = cfg.blocks_.Find(block.id()); v.has_value()) {
    for (const auto& block : v->PreviousBlocks()) {
      previous_blocks.push_back(block);
    }
  }
  return previous_blocks;
}

inline std::uint32_t VertexId(const AbstractMachineControlFlowGraph&,
                              AbstractMachineControlFlowGraph::BlockRef block) {
  return block.id();
}

}  // namespace lucid
