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
      if (auto* if_stmt = std::get_if<IfStmt>(&DerefStmt(stmt_ref))) {
        ProcessStmt(*if_stmt, block, end);
        FlushSubExprs(block, end);
        return;
      } else {
        ProcessSubExpr(stmt_ref, block, end);
        FlushSubExprs(block, end);
      }
    }

    graph_.get(block).next.push_back(graph_.last);
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
    for (ExprRef arg : expr.arguments) ProcessSubExpr(arg, block, end);
  }

  void ProcessExpr(const IntLitExpr& expr, BlockRef block, BlockRef end) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const BoolLitExpr& expr, BlockRef block, BlockRef end) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const IdentExpr& expr, BlockRef block, BlockRef end) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const BinaryOpExpr& expr, BlockRef block, BlockRef end) {
    ProcessSubExpr(expr.lhs, block, end);
    ProcessSubExpr(expr.rhs, block, end);
  }

  void ProcessExpr(const Type& expr, BlockRef block, BlockRef end) {
    // TODO: How to represent types in CFG?
  }

  void ProcessStmt(const ReturnStmt& stmt, BlockRef block, BlockRef end) {
    ProcessSubExpr(stmt.value, block, end);
  }

  void ProcessStmt(const Expr& expr, BlockRef block, BlockRef end) {
    std::visit([&](auto&& expr) { ProcessExpr(expr, block, end); }, expr);
  }

  void ProcessStmt(const VarDeclStmt& stmt, BlockRef block, BlockRef end) {
    ProcessSubExpr(stmt.init, block, end);
  }

  void ProcessStmt(const FuncDefStmt& stmt, BlockRef block, BlockRef end) {
    // TODO: Does it make sense to have a `FuncDefStmt` as a nested statement?
  }

  void ProcessStmt(const IfStmt& stmt, BlockRef block, BlockRef end) {
    auto then_block = AddBlock();
    BuildBlock(stmt.then_body, then_block, end);
    graph_.get(block).next.push_back(then_block);

    auto else_block = AddBlock();
    BuildBlock(stmt.else_body, else_block, end);
    graph_.get(block).next.push_back(else_block);

    pending_sub_exprs_.push(stmt.cond);
    graph_.get(block).branch_cond = stmt.cond;
  }

  void ProcessSubExpr(ExprRef expr_ref, BlockRef block, BlockRef end) {
    pending_sub_exprs_.push(expr_ref);
  }

  const Stmt& DerefStmt(StmtRef stmt_ref) { return arena_.get(stmt_ref); }

  const Arena<Stmt>& arena_;
  std::stack<StmtRef, std::vector<StmtRef>> pending_sub_exprs_;
  ControlFlowGraph graph_;
};

}  // namespace

ControlFlowGraph BuildControlFlowGraph(const Arena<Stmt>& arena,
                                       const FuncDefStmt& func) {
  return ControlFlowGraphBuilder(arena, func).Consume();
}

}  // namespace lucid
