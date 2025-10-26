#pragma once

#include <cstddef>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/successive_list.h"

namespace lucid {

// A graph that represents the control flow of a function.
struct ControlFlowGraph {
  struct Block;
  using BlockRef = Arena<Block>::Ref;

  // A null block reference.
  static constexpr BlockRef kNullBlockRef = Arena<Block>::kNullRef;

  // A sequence of expressions and an optional statement in evaluation order.
  struct Sequence {
    // Expressions in the sequence in evaluation order.
    std::vector<ExprRef> expressions;

    // Statement of the sequence.
    std::optional<StmtRef> stmt;
  };

  // Phi functions that represent variable definition join points when the
  // graph is converted to Static Single Assignment (SSA) form.
  struct Phi {
    // Name of the new variable defined by the Phi function.
    std::string name;

    // Type of the variable defined by the Phi function.
    TypeRef type_constraint;

    // Arguments to the Phi function, i.e. variables from previous blocks.
    std::vector<std::string> args;

    bool operator==(const Phi& other) const {
      return name == other.name && args == other.args;
    }

    friend void PrintTo(const Phi& phi, std::ostream* os) {
      *os << "Phi{name='" << phi.name << "', args=[";
      for (int i = 0; i < phi.args.size(); ++i) {
        if (i > 0) *os << ", ";
        *os << "'" << phi.args[i] << "'";
      }
      *os << "] }";
    }
  };

  // Represents a basic block in the control flow graph of a function.
  struct Block {
    // Id of the basic block.
    std::size_t id;

    // Phi functions that represent variable definition join points when the
    // graph is converted to Static Single Assignment (SSA) form.
    std::vector<Phi> phis;

    // A list of sequences in evaluation order.
    std::vector<Sequence> sequences;

    // References to the subsequent blocks.
    //
    // In the case of an `IfStmt` the first block in `next` will represent the
    // "then" branch and the second block will represent the "else" branch.
    std::vector<BlockRef> next;

    // References to the previous blocks.
    std::vector<BlockRef> preds;

    // Condition expression that determines the block in `next` that follows
    // this block.
    ExprRef branch_cond = Arena<Expr>::kNullRef;
  };

  // Adds `block` to the control flow graph and returns a reference to it.
  BlockRef add(Block block) { return blocks_.Add(std::move(block)); }

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

  // Name of the function.
  std::string_view func_name;

  // Parameters of the function.
  SuccessiveList<ParamRef> func_params;

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
// Requires:
// - `func` must be associated with `ctx`.
ControlFlowGraph BuildControlFlowGraph(const SyntaxContext& ctx,
                                       const FuncDefStmt& func);

}  // namespace lucid
