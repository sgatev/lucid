#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

// A graph that represents the control flow of a function.
struct ControlFlowGraph {
  struct Block;
  using BlockRef = Arena<Block>::Ref;

  // A null block reference.
  static constexpr BlockRef kNullBlockRef = Arena<Block>::kNullRef;

  // Represents a basic block in the control flow graph of a function.
  struct Block {
    // Id of the basic block.
    std::size_t id;

    // Statements in the block in evaluation order.
    std::vector<StmtRef> statements;

    // Reference to the subsequent blocks.
    //
    // In the case of an `IfStmt` the first block in `next` will represent the
    // "then" branch and the second block will represent the "else" branch.
    std::vector<BlockRef> next;

    // Condition expression that determines the block in `next` that follows
    // this block.
    ExprRef branch_cond = kNullBlockRef;
  };

  // Adds `block` to the control flow graph and returns a reference to it.
  BlockRef add(Block block) { return blocks_.add(std::move(block)); }

  // Returns the block in the control flow graph refererenced by `ref`.
  //
  // `ref` must not be `kNullBlockRef`.
  Block& get(BlockRef ref) { return blocks_.get(ref); }

  // Returns the block in the control flow graph refererenced by `ref`.
  //
  // `ref` must not be `kNullBlockRef`.
  const Block& get(BlockRef ref) const { return blocks_.get(ref); }

  // Returns an arena with all blocks that were added to the graph.
  const Arena<Block>& blocks() const { return blocks_; }

  // Name of the function.
  std::string_view func_name;

  // Parameters of the function.
  std::vector<FuncParam> func_params;

  // The first block in the control flow graph.
  BlockRef first = kNullBlockRef;

  // The last block in the control flow graph.
  BlockRef last = kNullBlockRef;

  // True if one of the statements in the graph involves a function call
  // expression.
  bool has_func_calls = false;

 private:
  Arena<Block> blocks_;
};

// Returns the control flow graph of `func`.
//
// All statements that are reachable from `func` must be allocated on `arena`.
ControlFlowGraph BuildControlFlowGraph(const Arena<Stmt>& arena,
                                       const FuncDefStmt& func);

}  // namespace lucid
