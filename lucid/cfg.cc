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
 public:
  ControlFlowGraphBuilder(const Arena<Stmt>& arena, const FuncDefStmt& func_def)
      : arena_(arena) {
    graph_.func_name = func_def.name;
    graph_.first = graph_.add(ControlFlowGraph::Block());
    graph_.last = graph_.add(ControlFlowGraph::Block());
    BuildBlock(func_def.body, graph_.get(graph_.first));
  }

  // Returns the constructed control flow graph.
  ControlFlowGraph Consume() && { return std::move(graph_); }

 private:
  void BuildBlock(const CompoundStmt& stmt, ControlFlowGraph::Block& block) {
    block.next = graph_.last;

    for (StmtRef stmt_ref : stmt.statements) {
      ProcessSubExpr(stmt_ref);

      while (!pending_sub_exprs_.empty()) {
        auto stmt_ref = pending_sub_exprs_.top();
        pending_sub_exprs_.pop();

        block.statements.push_back(stmt_ref);

        std::visit([this](auto&& stmt) { ProcessStmt(stmt); },
                   DerefStmt(stmt_ref));
      }

      std::reverse(block.statements.begin(), block.statements.end());
    }
  }

  void ProcessExpr(const FuncCallExpr& expr) {
    for (ExprRef arg : expr.arguments) ProcessSubExpr(arg);
  }

  void ProcessExpr(const IntLitExpr& expr) {
    // No sub-expressions to process.
  }

  void ProcessExpr(const IdentExpr& expr) {
    // No sub-expressions to process.
  }

  void ProcessStmt(const ReturnStmt& stmt) { ProcessSubExpr(stmt.value); }

  void ProcessStmt(const Expr& expr) {
    std::visit([this](auto&& expr) { ProcessExpr(expr); }, expr);
  }

  void ProcessStmt(const VarDeclStmt& stmt) { ProcessSubExpr(stmt.init); }

  void ProcessStmt(const FuncDefStmt& stmt) {
    // TODO: Does it make sense to have a `FuncDefStmt` as a nested statement?
  }

  void ProcessSubExpr(ExprRef expr_ref) { pending_sub_exprs_.push(expr_ref); }

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
