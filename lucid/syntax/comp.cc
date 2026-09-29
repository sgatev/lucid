#include "lucid/syntax/comp.h"

#include <expected>
#include <variant>

#include "lucid/core/container/hash_set.h"
#include "lucid/core/container/successive_list.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

// Checks an expression, and everything it is made of, against what can be
// worked out during compilation.
//
// `in_comp` says whether the expression stands inside one that is worked out
// there; a `comp` on the expression itself says so as well. `only_comp_calls`
// says that a call has to be to a comp function wherever it stands, which is
// what the body of a comp function is held to.
std::expected<void, CompError> CheckCompExpr(
    const HashSet<StringIndex::Ref>& comp_var_names, SyntaxContext& syn_ctx,
    ExprRef expr_ref, bool in_comp, bool only_comp_calls) {
  Expr& expr = syn_ctx.DerefExpr(expr_ref);

  // An expression is worked out during compilation where a `comp` stands on
  // it, and so is everything it is made of.
  const bool init_comp =
      in_comp ||
      std::visit([](const auto& expr) { return expr.is_comp; }, expr);
  if (init_comp) std::visit([](auto& expr) { expr.is_comp = true; }, expr);
  if (const auto* func_call_expr = std::get_if<FuncCallExpr>(&expr)) {
    StringIndex::Ref func_name = func_call_expr->func_name;
    const FuncDefStmt* func_def = syn_ctx.FindFuncDef(func_name);
    if (func_def == nullptr) return {};
    if ((init_comp || only_comp_calls) && !func_def->is_comp) {
      return std::unexpected(
          CompError("calling non-comp function not allowed in comp context"));
    }

    std::expected<void, CompError> result;
    for (const auto& arg : func_call_expr->args) {
      result = result.and_then([&] {
        return CheckCompExpr(comp_var_names, syn_ctx, arg, init_comp,
                             only_comp_calls);
      });
    }
    return result;
  } else if (const auto* index_expr = std::get_if<IndexExpr>(&expr)) {
    return CheckCompExpr(comp_var_names, syn_ctx, index_expr->base, init_comp,
                         only_comp_calls)
        .and_then([&] {
          return CheckCompExpr(comp_var_names, syn_ctx, index_expr->index,
                               init_comp, only_comp_calls);
        });
  } else if (const auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
    return CheckCompExpr(comp_var_names, syn_ctx, binary_op_expr->lhs,
                         init_comp, only_comp_calls)
        .and_then([&] {
          return CheckCompExpr(comp_var_names, syn_ctx, binary_op_expr->rhs,
                               init_comp, only_comp_calls);
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
                           /*in_comp=*/false, /*only_comp_calls=*/true);
    }
  } else if (const auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
    return CheckCompExpr(/*comp_var_names=*/{}, syn_ctx, var_assign_stmt->expr,
                         /*in_comp=*/false, /*only_comp_calls=*/true);
  } else if (const auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
    return CheckCompExpr(/*comp_var_names=*/{}, syn_ctx, return_stmt->value,
                         /*in_comp=*/false, /*only_comp_calls=*/true);
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

  const auto check = [&](ExprRef expr_ref) {
    return CheckCompExpr(comp_var_names, syn_ctx, expr_ref, /*in_comp=*/false,
                         /*only_comp_calls=*/false);
  };

  if (const auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
    return check(if_stmt->cond)
        .and_then([&] {
          return CheckCompVars(comp_var_names, syn_ctx, if_stmt->then_stmts);
        })
        .and_then([&] {
          return CheckCompVars(comp_var_names, syn_ctx, if_stmt->else_stmts);
        });
  } else if (const auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
    return CheckCompVars(comp_var_names, syn_ctx, loop_stmt->stmts);
  } else if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
    if (!var_decl_stmt->init.has_value()) return {};

    std::expected<void, CompError> res = check(*var_decl_stmt->init);
    if (!res.has_value()) return res;

    // What the variable holds is known during compilation where the whole
    // of what initializes it was worked out there and no write can reach it
    // afterwards. That is what lets another comp expression read it, and
    // what asks for the write into it to be worked out during compilation
    // as well.
    const Expr& init = syn_ctx.DerefExpr(*var_decl_stmt->init);
    const bool init_is_comp =
        std::visit([](const auto& expr) { return expr.is_comp; }, init);
    if (init_is_comp && !var_decl_stmt->is_mutable) {
      var_decl_stmt->is_comp = true;
      comp_var_names.Insert(var_decl_stmt->name);
    }
    return {};
  } else if (const auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
    return check(var_assign_stmt->expr);
  } else if (const auto* array_assign_stmt =
                 std::get_if<ArrayAssignStmt>(&stmt)) {
    return check(array_assign_stmt->index).and_then([&] {
      return check(array_assign_stmt->expr);
    });
  } else if (const auto* field_assign_stmt =
                 std::get_if<FieldAssignStmt>(&stmt)) {
    return check(field_assign_stmt->base).and_then([&] {
      return check(field_assign_stmt->expr);
    });
  } else if (const auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
    return check(return_stmt->value);
  } else if (const auto* do_stmt = std::get_if<DoStmt>(&stmt)) {
    return check(do_stmt->expr);
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
