#include "lucid/syntax/comp.h"

#include <expected>
#include <variant>

#include "lucid/core/container/hash_set.h"
#include "lucid/core/container/successive_list.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

std::expected<void, CompError> CheckCompExpr(
    const HashSet<StringIndex::Ref>& comp_var_names, SyntaxContext& syn_ctx,
    ExprRef expr_ref, bool init_comp) {
  Expr& expr = syn_ctx.DerefExpr(expr_ref);
  if (init_comp) std::visit([](auto& expr) { expr.is_comp = true; }, expr);
  if (const auto* func_call_expr = std::get_if<FuncCallExpr>(&expr)) {
    StringIndex::Ref func_name = func_call_expr->func_name;
    const FuncDefStmt* func_def = syn_ctx.FindFuncDef(func_name);
    if (func_def == nullptr) return {};
    if (!func_def->is_comp) {
      return std::unexpected(
          CompError("calling non-comp function not allowed in comp context"));
    }

    std::expected<void, CompError> result;
    for (const auto& arg : func_call_expr->args) {
      result = result.and_then([&] {
        return CheckCompExpr(comp_var_names, syn_ctx, arg, init_comp);
      });
    }
    return result;
  } else if (const auto* index_expr = std::get_if<IndexExpr>(&expr)) {
    return CheckCompExpr(comp_var_names, syn_ctx, index_expr->base, init_comp)
        .and_then([&] {
          return CheckCompExpr(comp_var_names, syn_ctx, index_expr->index,
                               init_comp);
        });
  } else if (const auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
    return CheckCompExpr(comp_var_names, syn_ctx, binary_op_expr->lhs,
                         init_comp)
        .and_then([&] {
          return CheckCompExpr(comp_var_names, syn_ctx, binary_op_expr->rhs,
                               init_comp);
        });
  } else if (const auto* ident_expr = std::get_if<IdentExpr>(&expr)) {
    if (init_comp && !comp_var_names.Contains(ident_expr->name)) {
      return std::unexpected(
          CompError("non-comp identifier not allowed in comp context"));
    }
  }
  return {};
}

std::expected<void, CompError> CheckCompFunc(SyntaxContext& syn_ctx,
                                             SuccessiveList<StmtRef> stmts);

std::expected<void, CompError> CheckCompFunc(SyntaxContext& syn_ctx,
                                             StmtRef stmt_ref) {
  const Stmt& stmt = syn_ctx.DerefStmt(stmt_ref);
  if (const auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
    return CheckCompFunc(syn_ctx, if_stmt->then_stmts).and_then([&] {
      return CheckCompFunc(syn_ctx, if_stmt->else_stmts);
    });
  } else if (const auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
    return CheckCompFunc(syn_ctx, loop_stmt->stmts);
  } else if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
    if (var_decl_stmt->init.has_value()) {
      return CheckCompExpr(/*comp_var_names=*/{}, syn_ctx, *var_decl_stmt->init,
                           /*init_comp=*/false);
    }
  } else if (const auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
    return CheckCompExpr(/*comp_var_names=*/{}, syn_ctx, var_assign_stmt->expr,
                         /*init_comp=*/false);
  } else if (const auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
    return CheckCompExpr(/*comp_var_names=*/{}, syn_ctx, return_stmt->value,
                         /*init_comp=*/false);
  } else if (std::holds_alternative<DoStmt>(stmt)) {
    return std::unexpected(
        CompError("do statement not allowed in comp context"));
  }
  return {};
}

std::expected<void, CompError> CheckCompFunc(SyntaxContext& syn_ctx,
                                             SuccessiveList<StmtRef> stmts) {
  for (StmtRef stmt_ref : stmts) {
    std::expected<void, CompError> res = CheckCompFunc(syn_ctx, stmt_ref);
    if (!res.has_value()) return res;
  }
  return {};
}

std::expected<void, CompError> CheckCompVars(
    HashSet<StringIndex::Ref>& comp_var_names, SyntaxContext& syn_ctx,
    SuccessiveList<StmtRef> stmts);

std::expected<void, CompError> CheckCompVars(
    HashSet<StringIndex::Ref>& comp_var_names, SyntaxContext& syn_ctx,
    StmtRef stmt_ref) {
  Stmt& stmt = syn_ctx.DerefStmt(stmt_ref);
  if (const auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
    return CheckCompVars(comp_var_names, syn_ctx, if_stmt->then_stmts)
        .and_then([&] {
          return CheckCompVars(comp_var_names, syn_ctx, if_stmt->else_stmts);
        });
  } else if (const auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
    return CheckCompVars(comp_var_names, syn_ctx, loop_stmt->stmts);
  } else if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
    if (var_decl_stmt->is_comp) {
      comp_var_names.Insert(var_decl_stmt->name);
      if (var_decl_stmt->init.has_value()) {
        return CheckCompExpr(comp_var_names, syn_ctx, *var_decl_stmt->init,
                             /*init_comp=*/true);
      }
    }
  }
  return {};
}

std::expected<void, CompError> CheckCompVars(
    HashSet<StringIndex::Ref>& comp_var_names, SyntaxContext& syn_ctx,
    SuccessiveList<StmtRef> stmts) {
  for (StmtRef stmt_ref : stmts) {
    std::expected<void, CompError> res =
        CheckCompVars(comp_var_names, syn_ctx, stmt_ref);
    if (!res.has_value()) return res;
  }
  return {};
}

}  // namespace

std::expected<void, CompError> CheckComp(SyntaxContext& syn_ctx,
                                         FuncDefStmt& stmt) {
  HashSet<StringIndex::Ref> comp_var_names;
  std::expected<void, CompError> res =
      CheckCompVars(comp_var_names, syn_ctx, stmt.stmts);
  if (res.has_value() && stmt.is_comp) {
    res = CheckCompFunc(syn_ctx, stmt.stmts);
  }
  return res;
}

}  // namespace lucid
