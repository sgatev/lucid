#pragma once

#include <optional>
#include <vector>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

// A graph that represents the control flow of a function.
struct ControlFlowGraph {
  struct Block;
  using BlockRef = Arena<Block>::Ref;

  // Represents a basic block in the control flow graph of a function.
  struct Block {
    // Statements in the block in evaluation order.
    std::vector<StmtRef> statements;

    // Reference to the subsequent block.
    std::optional<BlockRef> next;
  };

  // Adds `block` to the control flow graph and returns a reference to it.
  BlockRef add(Block block) { return blocks_.add(std::move(block)); }

  // Returns the block in the control flow graph refererenced by `ref`.
  Block& get(BlockRef ref) { return blocks_.get(ref); }

  // Returns the block in the control flow graph refererenced by `ref`.
  const Block& get(BlockRef ref) const { return blocks_.get(ref); }

  // The first block in the control flow graph.
  BlockRef first;

  // The last block in the control flow graph.
  BlockRef last;

 private:
  Arena<Block> blocks_;
};

// Returns the control flow graph of `func`.
//
// All statements that are reachable from `func` must be allocated on `arena`.
ControlFlowGraph BuildControlFlowGraph(const Arena<Stmt>& arena,
                                       const FuncDefStmt& func);

}  // namespace lucid
