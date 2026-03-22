#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <ranges>
#include <vector>

#include "lucid/ast.h"
#include "lucid/core/container/arena.h"
#include "lucid/string_index.h"
#include "lucid/successive_list.h"

namespace lucid {

// A graph that represents the control flow of a function.
struct ControlFlowGraph {
  struct Block;
  using BlockRef = Arena<Block>::Ref;
  using vertex_type = BlockRef;

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
    StringIndex::Ref name;

    // Type of the variable defined by the Phi function.
    TypeRef type_constraint;

    // Arguments to the Phi function, i.e. variables from previous blocks.
    std::vector<StringIndex::Ref> args;

    bool operator==(const Phi& other) const {
      return name == other.name && args == other.args;
    }
  };

  // Represents a basic block in the control flow graph of a function.
  struct Block {
    // Ref of the basic block.
    BlockRef ref;

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

  explicit ControlFlowGraph(StringIndex::Ref func_name)
      : func_name(func_name) {}

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
  StringIndex::Ref func_name;

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

inline std::size_t VertexCount(const ControlFlowGraph& cfg) {
  return cfg.blocks().Size();
}

inline std::vector<ControlFlowGraph::BlockRef> Vertices(
    const ControlFlowGraph& cfg) {
  std::vector<ControlFlowGraph::BlockRef> blocks;
  for (const auto& block : cfg.blocks()) blocks.push_back(block.ref);
  return blocks;
}

inline ControlFlowGraph::BlockRef SourceVertex(const ControlFlowGraph& cfg) {
  return cfg.first;
}

inline ControlFlowGraph::BlockRef SinkVertex(const ControlFlowGraph& cfg) {
  return cfg.last;
}

inline std::vector<ControlFlowGraph::BlockRef> NextVertices(
    const ControlFlowGraph& cfg, ControlFlowGraph::BlockRef block) {
  return cfg.get(block).next;
}

inline std::vector<ControlFlowGraph::BlockRef> PrevVertices(
    const ControlFlowGraph& cfg, ControlFlowGraph::BlockRef block) {
  return cfg.get(block).preds;
}

// Returns the control flow graph of `func`.
//
// Requires:
// - `func` must be associated with `ctx`.
ControlFlowGraph BuildControlFlowGraph(const SyntaxContext& ctx,
                                       const FuncDefStmt& func);

template <typename T>
class ControlFlowGraphAnalysis {
 public:
  using State = T::State;

  explicit ControlFlowGraphAnalysis(const ControlFlowGraph& cfg,
                                    const SyntaxContext& ctx)
      : cfg_(cfg), t_(ctx) {}

  State MakeInitial() { return t_.MakeInitial(); }

  State Transfer(State state, const ControlFlowGraph::BlockRef& block) {
    return std::ranges::fold_left(
        cfg_.get(block).sequences | std::views::reverse, state,
        std::bind_front(&T::Transfer, &t_));
  }

  State Join(State left, State right) { return t_.Join(left, right); }

 private:
  const ControlFlowGraph& cfg_;
  T t_;
};

}  // namespace lucid

namespace std {

template <>
struct hash<typename lucid::ControlFlowGraph::BlockRef> {
  size_t operator()(const lucid::ControlFlowGraph::BlockRef& ref) const {
    return hash<uint32_t>()(ref.id());
  }
};

}  // namespace std
