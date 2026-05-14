#include "lucid/syntax/comp.h"

#include <variant>
#include <vector>

#include "lucid/core/container/successive_list.h"
#include "lucid/core/functional/result.h"
#include "lucid/syntax/ast.h"

namespace lucid {
namespace {

Result<void, CompError> CheckCompExpr(const SyntaxContext& sctx,
                                      const std::vector<FuncDefStmt>& func_defs,
                                      ExprRef expr_ref) {
  const Expr& expr = sctx.DerefExpr(expr_ref);
  if (const auto* func_call_expr = std::get_if<FuncCallExpr>(&expr)) {
    StringIndex::Ref func_name = func_call_expr->func_name;
    for (const auto& func_def : func_defs) {
      if (func_def.name != func_name) continue;
      if (!func_def.is_comp) {
        return CompError(
            "calling non-comp function not allowed in comp context");
      }
    }
  } else if (const auto* index_expr = std::get_if<IndexExpr>(&expr)) {
    RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, index_expr->base));
    RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, index_expr->index));
  } else if (const auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
    RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, binary_op_expr->lhs));
    RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, binary_op_expr->rhs));
  }
  return {};
}

Result<void, CompError> CheckCompFunc(const SyntaxContext& sctx,
                                      const std::vector<FuncDefStmt>& func_defs,
                                      SuccessiveList<StmtRef> stmts) {
  for (StmtRef stmt_ref : stmts) {
    const Stmt& stmt = sctx.DerefStmt(stmt_ref);
    if (const auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
      RETURN_IF_ERROR(CheckCompFunc(sctx, func_defs, if_stmt->then_stmts));
      RETURN_IF_ERROR(CheckCompFunc(sctx, func_defs, if_stmt->else_stmts));
    } else if (const auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
      RETURN_IF_ERROR(CheckCompFunc(sctx, func_defs, loop_stmt->stmts));
    } else if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
      if (var_decl_stmt->init.has_value()) {
        RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, *var_decl_stmt->init));
      }
    } else if (const auto* var_assign_stmt =
                   std::get_if<VarAssignStmt>(&stmt)) {
      RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, var_assign_stmt->expr));
    } else if (const auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
      RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, return_stmt->value));
    } else if (std::holds_alternative<DoStmt>(stmt)) {
      return CompError("do statement not allowed in comp context");
    }
  }
  return {};
}

Result<void, CompError> CheckCompVars(const SyntaxContext& sctx,
                                      const std::vector<FuncDefStmt>& func_defs,
                                      SuccessiveList<StmtRef> stmts) {
  for (StmtRef stmt_ref : stmts) {
    const Stmt& stmt = sctx.DerefStmt(stmt_ref);
    if (const auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
      RETURN_IF_ERROR(CheckCompVars(sctx, func_defs, if_stmt->then_stmts));
      RETURN_IF_ERROR(CheckCompVars(sctx, func_defs, if_stmt->else_stmts));
    } else if (const auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
      RETURN_IF_ERROR(CheckCompVars(sctx, func_defs, loop_stmt->stmts));
    } else if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
      if (var_decl_stmt->is_comp && var_decl_stmt->init.has_value()) {
        RETURN_IF_ERROR(CheckCompExpr(sctx, func_defs, *var_decl_stmt->init));
      }
    }
  }
  return {};
}

}  // namespace

Result<void, CompError> CheckComp(const SyntaxContext& sctx,
                                  const std::vector<FuncDefStmt>& func_defs,
                                  const FuncDefStmt& stmt) {
  if (stmt.is_comp) RETURN_IF_ERROR(CheckCompFunc(sctx, func_defs, stmt.stmts));
  RETURN_IF_ERROR(CheckCompVars(sctx, func_defs, stmt.stmts));
  return {};
}

}  // namespace lucid
