#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "lucid/core/container/arena.h"
#include "lucid/core/container/successive_list.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {

// A graph that represents the control flow of a function at the syntax level.
struct SyntaxControlFlowGraph {
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

    bool operator==(const Phi& other) const = default;
  };

  // A reference to a phi functionn that can be dereferenced using an
  // `Arena<Phi>` object.
  using PhiRef = Arena<Phi>::Ref;

  // Represents a basic block in the control flow graph of a function.
  struct Block {
    // Ref of the basic block.
    BlockRef ref;

    // Phi functions that represent variable definition join points when the
    // graph is converted to Static Single Assignment (SSA) form.
    std::vector<PhiRef> phis;

    // A list of sequences in evaluation order.
    std::vector<Sequence> sequences;

    // References to the subsequent blocks.
    //
    // In the case of an `IfStmt` the first block in `next` will represent the
    // "then" branch and the second block will represent the "else" branch.
    std::vector<BlockRef> succs;

    // References to the previous blocks.
    std::vector<BlockRef> preds;

    // Condition expression that determines the block in `next` that follows
    // this block.
    ExprRef branch_cond = Arena<Expr>::kNullRef;
  };

  explicit SyntaxControlFlowGraph(StringIndex::Ref func_name)
      : func_name(func_name) {}

  // Adds `block` to the control flow graph and returns a reference to it.
  BlockRef add(Block block) { return blocks_.Add(std::move(block)); }

  // Returns the block in the control flow graph refererenced by `ref`.
  //
  // `ref` must not be `kNullBlockRef`.
  auto& get(this auto&& self, BlockRef ref) { return self.blocks_.Get(ref); }

  // Returns an arena with all blocks that were added to the graph.
  auto& blocks(this auto&& self) { return self.blocks_; }

  // Adds `phi` to the graph.
  PhiRef add(Phi phi) { return phis_.Add(std::move(phi)); }

  // Returns the phi function that `ref` refers to.
  auto& deref(this auto&& self, PhiRef ref) { return self.phis_.Get(ref); }

  // Name of the function.
  StringIndex::Ref func_name;

  // Parameters of the function.
  SuccessiveList<ParamRef> func_params;

  // Result type of the function.
  TypeRef func_result_type;

  // The first block in the control flow graph.
  BlockRef first = kNullBlockRef;

  // The last block in the control flow graph.
  BlockRef last = kNullBlockRef;

  // True if one of the statements in the graph involves a function call
  // expression.
  bool has_func_calls = false;

 private:
  Arena<Block> blocks_;
  Arena<Phi> phis_;
};

inline std::size_t VertexCount(const SyntaxControlFlowGraph& cfg) {
  return cfg.blocks().Size();
}

inline std::vector<SyntaxControlFlowGraph::BlockRef> Vertices(
    const SyntaxControlFlowGraph& cfg) {
  std::vector<SyntaxControlFlowGraph::BlockRef> blocks;
  for (const auto& block : cfg.blocks()) blocks.push_back(block.ref);
  return blocks;
}

inline SyntaxControlFlowGraph::BlockRef SourceVertex(
    const SyntaxControlFlowGraph& cfg) {
  return cfg.first;
}

inline SyntaxControlFlowGraph::BlockRef SinkVertex(
    const SyntaxControlFlowGraph& cfg) {
  return cfg.last;
}

inline std::span<const SyntaxControlFlowGraph::BlockRef> NextVertices(
    const SyntaxControlFlowGraph& cfg, SyntaxControlFlowGraph::BlockRef block) {
  return cfg.get(block).succs;
}

inline std::span<const SyntaxControlFlowGraph::BlockRef> PrevVertices(
    const SyntaxControlFlowGraph& cfg, SyntaxControlFlowGraph::BlockRef block) {
  return cfg.get(block).preds;
}

inline std::uint32_t VertexId(const SyntaxControlFlowGraph&,
                              SyntaxControlFlowGraph::BlockRef block) {
  return block.id();
}

// Returns the control flow graph of `func`.
//
// Requires:
// - `func` must be associated with `ctx`.
SyntaxControlFlowGraph BuildControlFlowGraph(SyntaxContext& ctx,
                                             const FuncDefStmt& func);

template <typename T>
class ControlFlowGraphAnalysis {
 public:
  using State = T::State;

  explicit ControlFlowGraphAnalysis(const SyntaxControlFlowGraph& cfg,
                                    const SyntaxContext& ctx)
      : cfg_(cfg), t_(ctx) {}

  State MakeInitial() { return t_.MakeInitial(); }

  State Transfer(State state, const SyntaxControlFlowGraph::BlockRef& block) {
    return std::ranges::fold_left(
        cfg_.get(block).sequences | std::views::reverse, state,
        std::bind_front(&T::Transfer, &t_));
  }

  State Join(State left, State right) { return t_.Join(left, right); }

 private:
  const SyntaxControlFlowGraph& cfg_;
  T t_;
};

inline std::size_t Hash(const SyntaxControlFlowGraph::BlockRef& ref) {
  return Hash(ref.id());
}

inline std::size_t Hash(const SyntaxControlFlowGraph::PhiRef& ref) {
  return Hash(ref.id());
}

}  // namespace lucid
