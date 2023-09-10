#include "lucid/type.h"

#include <string_view>
#include <variant>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

void PropagateType(Arena<Stmt>& arena, Expr& expr, std::string_view type);

void PropagateType(Arena<Stmt>& arena, FuncCallExpr& expr,
                   std::string_view type) {
  expr.type = type;
}

void PropagateType(Arena<Stmt>& arena, IntLitExpr& expr,
                   std::string_view type) {
  expr.type = type;
}

void PropagateType(Arena<Stmt>& arena, BoolLitExpr& expr,
                   std::string_view type) {
  expr.type = type;
}

void PropagateType(Arena<Stmt>& arena, IdentExpr& expr, std::string_view type) {
  expr.type = type;
}

void PropagateType(Arena<Stmt>& arena, BinaryOpExpr& expr,
                   std::string_view type) {
  expr.type = type;

  auto& lhs = std::get<Expr>(arena.get(expr.lhs));
  PropagateType(arena, lhs, type);

  auto& rhs = std::get<Expr>(arena.get(expr.rhs));
  PropagateType(arena, rhs, type);
}

void PropagateType(Arena<Stmt>& arena, Expr& expr, std::string_view type) {
  std::visit([&arena, type](auto& expr) { PropagateType(arena, expr, type); },
             expr);
}

void DeduceStmtTypes(Arena<Stmt>& arena, Expr& stmt,
                     std::string_view result_type) {}

void DeduceStmtTypes(Arena<Stmt>& arena, VarDeclStmt& stmt,
                     std::string_view result_type) {
  auto& init = std::get<Expr>(arena.get(stmt.init));
  PropagateType(arena, init, stmt.type);
}

void DeduceStmtTypes(Arena<Stmt>& arena, FuncDefStmt& stmt,
                     std::string_view result_type) {}

void DeduceStmtTypes(Arena<Stmt>& arena, ReturnStmt& stmt,
                     std::string_view result_type) {
  auto& value = std::get<Expr>(arena.get(stmt.value));
  PropagateType(arena, value, result_type);
}

void DeduceStmtTypes(Arena<Stmt>& arena, IfStmt& stmt,
                     std::string_view result_type) {
  for (auto stmt_ref : stmt.then_body.statements) {
    auto& stmt = arena.get(stmt_ref);
    std::visit([&arena, result_type](
                   auto& stmt) { DeduceStmtTypes(arena, stmt, result_type); },
               stmt);
  }
  for (auto stmt_ref : stmt.else_body.statements) {
    auto& stmt = arena.get(stmt_ref);
    std::visit([&arena, result_type](
                   auto& stmt) { DeduceStmtTypes(arena, stmt, result_type); },
               stmt);
  }
}

}  // namespace

void DeduceTypes(Arena<Stmt>& arena, FuncDefStmt& func_def) {
  for (auto stmt_ref : func_def.body.statements) {
    auto& stmt = arena.get(stmt_ref);
    std::visit(
        [&arena, &func_def](auto& stmt) {
          DeduceStmtTypes(arena, stmt, func_def.result_type);
        },
        stmt);
  }
}

}  // namespace lucid
