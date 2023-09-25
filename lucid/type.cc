#include "lucid/type.h"

#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

std::unordered_map<std::string_view, FuncType> ExtractFuncTypes(
    const std::vector<FuncDefStmt>& func_defs) {
  std::unordered_map<std::string_view, FuncType> func_types;
  for (const auto& func_def : func_defs) {
    func_types[func_def.name] = {
        .result_type = func_def.result_type,
        .parameters = func_def.parameters,
    };
  }
  return func_types;
}

std::optional<TypeError> InferExprTypes(
    Arena<Stmt>& arena,
    const std::unordered_map<std::string_view, FuncType>& func_types,
    FuncDefStmt& func_def) {
  std::unordered_map<ExprRef, ExprRef> expr_from_expr;
  std::unordered_map<ExprRef, std::string_view> expr_from_type;
  std::unordered_map<std::string_view, std::string_view> ident_from_type;

  for (auto param : func_def.parameters) {
    ident_from_type[param.name] = param.type;
  }

  std::vector<StmtRef> pending_stmts;
  for (auto stmt : std::ranges::reverse_view(func_def.body.statements)) {
    pending_stmts.push_back(stmt);
  }

  std::vector<ExprRef> pending_exprs;
  while (!pending_stmts.empty()) {
    auto stmt_ref = pending_stmts.back();
    auto& stmt = arena.get(stmt_ref);
    pending_stmts.pop_back();

    if (auto* cstmt = std::get_if<IfStmt>(&stmt)) {
      for (auto stmt : std::ranges::reverse_view(cstmt->else_body.statements)) {
        pending_stmts.push_back(stmt);
      }
      for (auto stmt : std::ranges::reverse_view(cstmt->then_body.statements)) {
        pending_stmts.push_back(stmt);
      }

      expr_from_type[cstmt->cond] = "Bool";

      pending_exprs.push_back(cstmt->cond);
    } else if (auto* cstmt = std::get_if<ReturnStmt>(&stmt)) {
      expr_from_type[cstmt->value] = func_def.result_type;

      pending_exprs.push_back(cstmt->value);
    } else if (auto* cstmt = std::get_if<VarDeclStmt>(&stmt)) {
      ident_from_type[cstmt->name] = cstmt->type;
      expr_from_type[cstmt->init] = cstmt->type;

      pending_exprs.push_back(cstmt->init);
    } else if (auto* cstmt = std::get_if<Expr>(&stmt)) {
      pending_exprs.push_back(stmt_ref);
    }

    while (!pending_exprs.empty()) {
      auto expr_ref = pending_exprs.back();
      auto& expr = std::get<Expr>(arena.get(expr_ref));
      pending_exprs.pop_back();

      if (auto* cexpr = std::get_if<FuncCallExpr>(&expr)) {
        const auto& func_type = func_types.at(cexpr->func_name);
        for (int i = 0; i < cexpr->arguments.size(); ++i) {
          const auto& arg = cexpr->arguments[i];
          expr_from_type[arg] = func_type.parameters[i].type;
          pending_exprs.push_back(arg);
        }
      } else if (auto* cexpr = std::get_if<BoolLitExpr>(&expr)) {
        if (auto it = expr_from_type.find(expr_ref);
            it != expr_from_type.end() && it->second != "Bool") {
          return TypeError(std::string("Bool literal is not of type ") +
                           std::string(it->second));
        }
        expr_from_type[expr_ref] = "Bool";
      } else if (auto* cexpr = std::get_if<IdentExpr>(&expr)) {
        if (auto it = expr_from_type.find(expr_ref);
            it != expr_from_type.end() &&
            it->second != ident_from_type[cexpr->name]) {
          return TypeError(
              std::string("Identifier '") + std::string(cexpr->name) +
              std::string("' is not of type ") + std::string(it->second));
        }
        expr_from_type[expr_ref] = ident_from_type[cexpr->name];
      } else if (auto* cexpr = std::get_if<BinaryOpExpr>(&expr)) {
        if (cexpr->op == BinaryOp::Eq) {
          expr_from_expr[cexpr->rhs] = cexpr->lhs;
          expr_from_expr[cexpr->lhs] = cexpr->rhs;
        } else {
          expr_from_expr[cexpr->rhs] = cexpr->lhs;
          expr_from_expr[expr_ref] = cexpr->rhs;
          expr_from_expr[cexpr->lhs] = expr_ref;
        }

        pending_exprs.push_back(cexpr->rhs);
        pending_exprs.push_back(cexpr->lhs);
      }
    }
  }

  while (true) {
    std::unordered_map<ExprRef, ExprRef> next_expr_from_expr;
    for (auto [lhs, rhs] : expr_from_expr) {
      if (auto it = expr_from_type.find(rhs); it != expr_from_type.end()) {
        expr_from_type[lhs] = it->second;
      } else {
        next_expr_from_expr[lhs] = rhs;
      }
    }
    if (next_expr_from_expr.size() == expr_from_expr.size()) break;
    expr_from_expr = std::move(next_expr_from_expr);
  }

  for (auto [expr_ref, type] : expr_from_type) {
    auto& expr = std::get<Expr>(arena.get(expr_ref));
    if (auto* cexpr = std::get_if<FuncCallExpr>(&expr)) {
      cexpr->type = type;
    } else if (auto* cexpr = std::get_if<BoolLitExpr>(&expr)) {
      cexpr->type = type;
    } else if (auto* cexpr = std::get_if<IntLitExpr>(&expr)) {
      cexpr->type = type;
    } else if (auto* cexpr = std::get_if<IdentExpr>(&expr)) {
      cexpr->type = type;
    } else if (auto* cexpr = std::get_if<BinaryOpExpr>(&expr)) {
      cexpr->type = type;
    }
  }

  return std::nullopt;
}

}  // namespace lucid
