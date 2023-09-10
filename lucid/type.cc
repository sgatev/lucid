#include "lucid/type.h"

#include <optional>
#include <string_view>
#include <variant>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

std::optional<std::string> PropagateType(Arena<Stmt>& arena, Expr& expr,
                                         std::string_view type);

std::optional<std::string> PropagateType(Arena<Stmt>& arena, FuncCallExpr& expr,
                                         std::string_view type) {
  expr.type = type;
  return std::nullopt;
}

std::optional<std::string> PropagateType(Arena<Stmt>& arena, IntLitExpr& expr,
                                         std::string_view type) {
  expr.type = type;
  return std::nullopt;
}

std::optional<std::string> PropagateType(Arena<Stmt>& arena, BoolLitExpr& expr,
                                         std::string_view type) {
  if (type != "Bool") {
    return std::string("Bool literal is not of type ") + std::string(type);
  }
  expr.type = type;
  return std::nullopt;
}

std::optional<std::string> PropagateType(Arena<Stmt>& arena, IdentExpr& expr,
                                         std::string_view type) {
  expr.type = type;
  return std::nullopt;
}

std::optional<std::string> PropagateType(Arena<Stmt>& arena, BinaryOpExpr& expr,
                                         std::string_view type) {
  expr.type = type;

  auto& lhs = std::get<Expr>(arena.get(expr.lhs));
  if (auto error = PropagateType(arena, lhs, type); error.has_value()) {
    return error;
  }

  auto& rhs = std::get<Expr>(arena.get(expr.rhs));
  if (auto error = PropagateType(arena, rhs, type); error.has_value()) {
    return error;
  }

  return std::nullopt;
}

std::optional<std::string> PropagateType(Arena<Stmt>& arena, Expr& expr,
                                         std::string_view type) {
  return std::visit(
      [&arena, type](auto& expr) { return PropagateType(arena, expr, type); },
      expr);
}

std::optional<std::string> DeduceStmtTypes(Arena<Stmt>& arena, Expr& stmt,
                                           std::string_view result_type) {
  return std::nullopt;
}

std::optional<std::string> DeduceStmtTypes(Arena<Stmt>& arena,
                                           VarDeclStmt& stmt,
                                           std::string_view result_type) {
  auto& init = std::get<Expr>(arena.get(stmt.init));
  return PropagateType(arena, init, stmt.type);
}

std::optional<std::string> DeduceStmtTypes(Arena<Stmt>& arena,
                                           FuncDefStmt& stmt,
                                           std::string_view result_type) {
  return std::nullopt;
}

std::optional<std::string> DeduceStmtTypes(Arena<Stmt>& arena, ReturnStmt& stmt,
                                           std::string_view result_type) {
  auto& value = std::get<Expr>(arena.get(stmt.value));
  return PropagateType(arena, value, result_type);
}

std::optional<std::string> DeduceStmtTypes(Arena<Stmt>& arena, IfStmt& stmt,
                                           std::string_view result_type) {
  for (auto stmt_ref : stmt.then_body.statements) {
    auto& stmt = arena.get(stmt_ref);
    auto error = std::visit(
        [&arena, result_type](auto& stmt) {
          return DeduceStmtTypes(arena, stmt, result_type);
        },
        stmt);
    if (error.has_value()) return error;
  }
  for (auto stmt_ref : stmt.else_body.statements) {
    auto& stmt = arena.get(stmt_ref);
    auto error = std::visit(
        [&arena, result_type](auto& stmt) {
          return DeduceStmtTypes(arena, stmt, result_type);
        },
        stmt);
    if (error.has_value()) return error;
  }
  return std::nullopt;
}

}  // namespace

std::optional<std::string> DeduceTypes(Arena<Stmt>& arena,
                                       FuncDefStmt& func_def) {
  for (auto stmt_ref : func_def.body.statements) {
    auto& stmt = arena.get(stmt_ref);
    auto error = std::visit(
        [&arena, &func_def](auto& stmt) {
          return DeduceStmtTypes(arena, stmt, func_def.result_type);
        },
        stmt);
    if (error.has_value()) return error;
  }
  return std::nullopt;
}

}  // namespace lucid
