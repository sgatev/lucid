#include "lucid/syntax/comp.h"

#include <expected>
#include <ranges>
#include <variant>
#include <vector>

#include "lucid/core/container/successive_list.h"
#include "lucid/syntax/ast.h"

namespace lucid {
namespace {

std::expected<void, CompError> CheckCompExpr(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    ExprRef expr_ref) {
  const Expr& expr = syn_ctx.DerefExpr(expr_ref);
  if (const auto* func_call_expr = std::get_if<FuncCallExpr>(&expr)) {
    StringIndex::Ref func_name = func_call_expr->func_name;
    for (const auto& func_def : func_defs) {
      if (func_def.name != func_name) continue;
      if (!func_def.is_comp) {
        return std::unexpected(
            CompError("calling non-comp function not allowed in comp context"));
      }
    }
  } else if (const auto* index_expr = std::get_if<IndexExpr>(&expr)) {
    return CheckCompExpr(syn_ctx, func_defs, index_expr->base).and_then([&] {
      return CheckCompExpr(syn_ctx, func_defs, index_expr->index);
    });
  } else if (const auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
    return CheckCompExpr(syn_ctx, func_defs, binary_op_expr->lhs).and_then([&] {
      return CheckCompExpr(syn_ctx, func_defs, binary_op_expr->rhs);
    });
  }
  return {};
}

std::expected<void, CompError> CheckCompFunc(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    SuccessiveList<StmtRef> stmts);

std::expected<void, CompError> CheckCompFunc(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    StmtRef stmt_ref) {
  const Stmt& stmt = syn_ctx.DerefStmt(stmt_ref);
  if (const auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
    return CheckCompFunc(syn_ctx, func_defs, if_stmt->then_stmts).and_then([&] {
      return CheckCompFunc(syn_ctx, func_defs, if_stmt->else_stmts);
    });
  } else if (const auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
    return CheckCompFunc(syn_ctx, func_defs, loop_stmt->stmts);
  } else if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
    if (var_decl_stmt->init.has_value()) {
      return CheckCompExpr(syn_ctx, func_defs, *var_decl_stmt->init);
    }
  } else if (const auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
    return CheckCompExpr(syn_ctx, func_defs, var_assign_stmt->expr);
  } else if (const auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
    return CheckCompExpr(syn_ctx, func_defs, return_stmt->value);
  } else if (std::holds_alternative<DoStmt>(stmt)) {
    return std::unexpected(
        CompError("do statement not allowed in comp context"));
  }
  return {};
}

std::expected<void, CompError> CheckCompFunc(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    SuccessiveList<StmtRef> stmts) {
  for (StmtRef stmt_ref : stmts) {
    std::expected<void, CompError> res =
        CheckCompFunc(syn_ctx, func_defs, stmt_ref);
    if (!res.has_value()) return res;
  }
  return {};
}

std::expected<void, CompError> CheckCompVars(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    SuccessiveList<StmtRef> stmts);

std::expected<void, CompError> CheckCompVars(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    StmtRef stmt_ref) {
  const Stmt& stmt = syn_ctx.DerefStmt(stmt_ref);
  if (const auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
    return CheckCompVars(syn_ctx, func_defs, if_stmt->then_stmts).and_then([&] {
      return CheckCompVars(syn_ctx, func_defs, if_stmt->else_stmts);
    });
  } else if (const auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
    return CheckCompVars(syn_ctx, func_defs, loop_stmt->stmts);
  } else if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
    if (var_decl_stmt->is_comp && var_decl_stmt->init.has_value()) {
      return CheckCompExpr(syn_ctx, func_defs, *var_decl_stmt->init);
    }
  }
  return {};
}

std::expected<void, CompError> CheckCompVars(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    SuccessiveList<StmtRef> stmts) {
  for (StmtRef stmt_ref : stmts) {
    std::expected<void, CompError> res =
        CheckCompVars(syn_ctx, func_defs, stmt_ref);
    if (!res.has_value()) return res;
  }
  return {};
}

}  // namespace

std::expected<void, CompError> CheckComp(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    const FuncDefStmt& stmt) {
  std::expected<void, CompError> res =
      CheckCompVars(syn_ctx, func_defs, stmt.stmts);
  if (res.has_value() && stmt.is_comp) {
    res = CheckCompFunc(syn_ctx, func_defs, stmt.stmts);
  }
  return res;
}

}  // namespace lucid
