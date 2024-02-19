#include "lucid/cfg.h"

#include <algorithm>
#include <memory>
#include <stack>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/ast.h"

namespace lucid {
namespace {

// Builds the control flow graph of a function.
class ControlFlowGraphBuilder {
  using BlockRef = ControlFlowGraph::BlockRef;

 public:
  ControlFlowGraphBuilder(const Arena<Stmt>& arena, const FuncDefStmt& func_def)
      : arena_(arena) {
    graph_.func_name = func_def.name;
    graph_.func_params = func_def.parameters;
    graph_.first = AddBlock();
    graph_.last = AddBlock();
    BuildBlock(func_def.body, graph_.first, graph_.last);
  }

  // Returns the constructed control flow graph.
  ControlFlowGraph Consume() && { return std::move(graph_); }

 private:
  BlockRef AddBlock() {
    ControlFlowGraph::Block block;
    block.id = graph_.blocks().size();
    return graph_.add(std::move(block));
  }

  void BuildBlock(const CompoundStmt& stmt, BlockRef block, BlockRef end) {
    for (StmtRef stmt_ref : stmt.statements) {
      if (auto* loop_stmt = std::get_if<LoopStmt>(&DerefStmt(stmt_ref))) {
        auto post_loop_block = AddBlock();
        ProcessStmt(*loop_stmt, block, post_loop_block);
        FlushSubExprs(block, post_loop_block);
        block = post_loop_block;
      } else if (auto* if_stmt = std::get_if<IfStmt>(&DerefStmt(stmt_ref))) {
        auto post_if_block = AddBlock();
        ProcessStmt(*if_stmt, block, post_if_block);
        FlushSubExprs(block, post_if_block);
        block = post_if_block;
      } else {
        ProcessSubExpr(stmt_ref, block);
        FlushSubExprs(block, end);

        if (std::holds_alternative<ReturnStmt>(DerefStmt(stmt_ref))) {
          graph_.get(block).next.push_back(graph_.last);
          return;
        }

        if (std::holds_alternative<BreakStmt>(DerefStmt(stmt_ref))) {
          graph_.get(block).next.push_back(post_loop_blocks_.top());
          return;
        }
      }
    }

    graph_.get(block).next.push_back(end);
  }

  void FlushSubExprs(BlockRef block, BlockRef end) {
    auto expr_begin = graph_.get(block).statements.size();

    while (!pending_sub_exprs_.empty()) {
      auto stmt_ref = pending_sub_exprs_.top();
      pending_sub_exprs_.pop();

      graph_.get(block).statements.push_back(stmt_ref);

      std::visit([&](auto&& stmt) { ProcessStmt(stmt, block, end); },
                 DerefStmt(stmt_ref));
    }

    std::reverse(graph_.get(block).statements.begin() + expr_begin,
                 graph_.get(block).statements.end());
  }

  void ProcessExpr(const FuncCallExpr& expr, BlockRef block, BlockRef end) {
    graph_.has_func_calls = true;
    for (ExprRef arg : expr.arguments) ProcessSubExpr(arg, block);
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
    ProcessSubExpr(expr.base, block);
    ProcessSubExpr(expr.index, block);
  }

  void ProcessExpr(const BinaryOpExpr& expr, BlockRef block, BlockRef end) {
    ProcessSubExpr(expr.lhs, block);
    ProcessSubExpr(expr.rhs, block);
  }

  void ProcessExpr(const Type& expr, BlockRef block, BlockRef end) {
    // TODO: How to represent types in CFG?
  }

  void ProcessStmt(const ReturnStmt& stmt, BlockRef block, BlockRef end) {
    ProcessSubExpr(stmt.value, block);
  }

  void ProcessStmt(const DoStmt& stmt, BlockRef block, BlockRef end) {
    ProcessSubExpr(stmt.expr, block);
  }

  void ProcessStmt(const Expr& expr, BlockRef block, BlockRef end) {
    std::visit([&](auto&& expr) { ProcessExpr(expr, block, end); }, expr);
  }

  void ProcessStmt(const VarDeclStmt& stmt, BlockRef block, BlockRef end) {
    ProcessSubExpr(stmt.init, block);
  }

  void ProcessStmt(const VarAssignStmt& stmt, BlockRef block, BlockRef end) {
    ProcessSubExpr(stmt.expr, block);
  }

  void ProcessStmt(const ArrayAssignStmt& stmt, BlockRef block, BlockRef end) {
    ProcessSubExpr(stmt.index, block);
    ProcessSubExpr(stmt.expr, block);
  }

  void ProcessStmt(const FuncDefStmt& stmt, BlockRef block, BlockRef end) {
    // TODO: Does it make sense to have a `FuncDefStmt` as a nested statement?
  }

  void ProcessStmt(const LoopStmt& stmt, BlockRef block, BlockRef end) {
    post_loop_blocks_.push(end);

    auto loop_block = AddBlock();
    BuildBlock(stmt.body, loop_block, loop_block);
    graph_.get(block).next.push_back(loop_block);

    post_loop_blocks_.pop();
  }

  void ProcessStmt(const BreakStmt& stmt, BlockRef block, BlockRef end) {}

  void ProcessStmt(const IfStmt& stmt, BlockRef block, BlockRef end) {
    auto then_block = AddBlock();
    BuildBlock(stmt.then_body, then_block, end);
    graph_.get(block).next.push_back(then_block);

    if (stmt.else_body.statements.empty()) {
      graph_.get(block).next.push_back(end);
    } else {
      auto else_block = AddBlock();
      BuildBlock(stmt.else_body, else_block, end);
      graph_.get(block).next.push_back(else_block);
    }

    pending_sub_exprs_.push(stmt.cond);
    graph_.get(block).branch_cond = stmt.cond;
  }

  void ProcessSubExpr(ExprRef expr_ref, BlockRef block) {
    pending_sub_exprs_.push(expr_ref);
  }

  const Stmt& DerefStmt(StmtRef stmt_ref) { return arena_.get(stmt_ref); }

  const Arena<Stmt>& arena_;
  std::stack<StmtRef, std::vector<StmtRef>> pending_sub_exprs_;
  ControlFlowGraph graph_;
  std::stack<BlockRef> post_loop_blocks_;
};

}  // namespace

ControlFlowGraph BuildControlFlowGraph(const Arena<Stmt>& arena,
                                       const FuncDefStmt& func) {
  return ControlFlowGraphBuilder(arena, func).Consume();
}

}  // namespace lucid
