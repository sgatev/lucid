#include "lucid/syntax/cfg.h"

#include <algorithm>
#include <stack>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/core/container/successive_list.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

// Builds the control flow graph of a function.
class ControlFlowGraphBuilder {
  using BlockRef = SyntaxControlFlowGraph::BlockRef;
  using Sequence = SyntaxControlFlowGraph::Sequence;

 public:
  ControlFlowGraphBuilder(const SyntaxContext& ctx, const FuncDefStmt& func_def)
      : ctx_(ctx), graph_(func_def.name) {
    graph_.func_params = func_def.params;
    graph_.func_result_type = func_def.result_type;
    graph_.first = AddBlock();
    graph_.last = AddBlock();
    BuildBlock(func_def.stmts, graph_.first, graph_.last);
  }

  // Returns the constructed control flow graph.
  SyntaxControlFlowGraph Consume() && { return std::move(graph_); }

 private:
  BlockRef AddBlock() {
    SyntaxControlFlowGraph::Block block;
    auto ref = graph_.add(std::move(block));
    graph_.get(ref).ref = ref;
    return ref;
  }

  bool BuildBlock(const SuccessiveList<StmtRef>& stmts, BlockRef block,
                  BlockRef end) {
    bool should_connect = true;
    for (StmtRef stmt_ref : stmts) {
      if (auto* loop_stmt = std::get_if<LoopStmt>(&ctx_.DerefStmt(stmt_ref))) {
        auto post_loop_block = AddBlock();

        post_loop_blocks_.push(post_loop_block);

        auto loop_block = AddBlock();
        should_connect = BuildBlock(loop_stmt->stmts, loop_block, loop_block);
        graph_.get(block).succs.push_back(loop_block);
        graph_.get(loop_block).preds.push_back(block);

        post_loop_blocks_.pop();

        block = post_loop_block;
      } else if (auto* if_stmt =
                     std::get_if<IfStmt>(&ctx_.DerefStmt(stmt_ref))) {
        auto& seq = graph_.get(block).sequences.emplace_back();
        auto post_if_block = AddBlock();

        auto then_block = AddBlock();
        bool then_continues =
            BuildBlock(if_stmt->then_stmts, then_block, post_if_block);
        graph_.get(block).succs.push_back(then_block);
        graph_.get(then_block).preds.push_back(block);

        if (if_stmt->else_stmts.size() == 0) {
          graph_.get(block).succs.push_back(post_if_block);
          graph_.get(post_if_block).preds.push_back(block);
        } else {
          auto else_block = AddBlock();
          bool else_continues =
              BuildBlock(if_stmt->else_stmts, else_block, post_if_block);
          graph_.get(block).succs.push_back(else_block);
          graph_.get(else_block).preds.push_back(block);

          should_connect = then_continues || else_continues;
        }

        pending_sub_exprs_.push(if_stmt->cond);
        graph_.get(block).branch_cond = if_stmt->cond;

        FlushSubExprs(seq, block, post_if_block);
        block = post_if_block;
      } else {
        auto& seq = graph_.get(block).sequences.emplace_back();
        seq.stmt = stmt_ref;

        std::visit([&](auto&& stmt) { ProcessStmt(stmt, seq, block, end); },
                   ctx_.DerefStmt(stmt_ref));

        FlushSubExprs(seq, block, end);

        if (std::holds_alternative<ReturnStmt>(ctx_.DerefStmt(stmt_ref))) {
          graph_.get(block).succs.push_back(graph_.last);
          graph_.get(graph_.last).preds.push_back(block);
          return false;
        }

        if (std::holds_alternative<BreakStmt>(ctx_.DerefStmt(stmt_ref))) {
          graph_.get(block).succs.push_back(post_loop_blocks_.top());
          graph_.get(post_loop_blocks_.top()).preds.push_back(block);
          return false;
        }
      }
    }

    if (should_connect) {
      graph_.get(block).succs.push_back(end);
      graph_.get(end).preds.push_back(block);
    }

    return true;
  }

  void FlushSubExprs(Sequence& seq, BlockRef block, BlockRef end) {
    while (!pending_sub_exprs_.empty()) {
      auto expr_ref = pending_sub_exprs_.top();
      pending_sub_exprs_.pop();

      seq.expressions.push_back(expr_ref);

      std::visit([&](auto&& expr) { ProcessExpr(expr, block, end); },
                 ctx_.DerefExpr(expr_ref));
    }

    std::reverse(seq.expressions.begin(), seq.expressions.end());
  }

  void ProcessExpr(const FuncCallExpr& expr, BlockRef block, BlockRef end) {
    graph_.has_func_calls = true;
    for (ExprRef arg : expr.args) ProcessSubExpr(arg);
  }

  void ProcessExpr(const IntLitExpr& expr, BlockRef block, BlockRef end) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const BoolLitExpr& expr, BlockRef block, BlockRef end) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const StringLitExpr& expr, BlockRef block, BlockRef end) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const IdentExpr& expr, BlockRef block, BlockRef end) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const IndexExpr& expr, BlockRef block, BlockRef end) {
    ProcessSubExpr(expr.base);
    ProcessSubExpr(expr.index);
  }

  void ProcessExpr(const FieldAccessExpr& expr, BlockRef block, BlockRef end) {
    ProcessSubExpr(expr.base);
  }

  void ProcessExpr(const BinaryOpExpr& expr, BlockRef block, BlockRef end) {
    ProcessSubExpr(expr.lhs);
    ProcessSubExpr(expr.rhs);
  }

  void ProcessExpr(const Type& expr, BlockRef block, BlockRef end) {
    // TODO: How to represent types in CFG?
  }

  void ProcessStmt(const ReturnStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    ProcessSubExpr(stmt.value);
  }

  void ProcessStmt(const DoStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    ProcessSubExpr(stmt.expr);
  }

  void ProcessStmt(const VarDeclStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    if (stmt.init.has_value()) ProcessSubExpr(*stmt.init);
  }

  void ProcessStmt(const VarAssignStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    ProcessSubExpr(stmt.expr);
  }

  void ProcessStmt(const ArrayAssignStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    ProcessSubExpr(stmt.index);
    ProcessSubExpr(stmt.expr);
  }

  void ProcessStmt(const FieldAssignStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    ProcessSubExpr(stmt.base);
    ProcessSubExpr(stmt.expr);
  }

  void ProcessStmt(const FuncDefStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    // TODO: Does it make sense to have a `FuncDefStmt` as a nested statement?
  }

  void ProcessStmt(const TypeDefStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {
    // TODO: Does it make sense to have a `TypeDefStmt` as a nested statement?
  }

  void ProcessStmt(const LoopStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {}

  void ProcessStmt(const BreakStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {}

  void ProcessStmt(const IfStmt& stmt, Sequence& seq, BlockRef block,
                   BlockRef end) {}

  void ProcessSubExpr(ExprRef expr_ref) { pending_sub_exprs_.push(expr_ref); }

  const SyntaxContext& ctx_;
  std::stack<ExprRef, std::vector<ExprRef>> pending_sub_exprs_;
  SyntaxControlFlowGraph graph_;
  std::stack<BlockRef> post_loop_blocks_;
};

}  // namespace

SyntaxControlFlowGraph BuildControlFlowGraph(const SyntaxContext& ctx,
                                             const FuncDefStmt& func) {
  return ControlFlowGraphBuilder(ctx, func).Consume();
}

}  // namespace lucid
