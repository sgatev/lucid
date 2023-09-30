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
namespace {

// Pushes all elements of `view` to the back of `out`.
template <typename T, std::ranges::view V>
void AppendRange(std::vector<T>& out, V view) {
  for (auto element : view) out.push_back(element);
}

}  // namespace

std::unordered_map<std::string_view, FuncType> ExtractFuncTypes(
    Arena<Stmt>& arena, const std::vector<FuncDefStmt>& func_defs) {
  std::unordered_map<std::string_view, FuncType> func_types;
  for (const auto& func_def : func_defs) {
    const auto& result_type = std::get<BasicType>(
        std::get<Type>(std::get<Expr>(arena.get(func_def.result_type))));
    func_types[func_def.name] = {
        .result_type = result_type.name,
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

  for (const auto& param : func_def.parameters) {
    const auto& param_type = std::get<BasicType>(
        std::get<Type>(std::get<Expr>(arena.get(param.type))));
    ident_from_type[param.name] = param_type.name;
  }

  std::vector<StmtRef> pending_stmts;
  AppendRange(pending_stmts,
              std::ranges::reverse_view(func_def.body.statements));

  std::vector<ExprRef> pending_exprs;
  while (!pending_stmts.empty()) {
    auto stmt_ref = pending_stmts.back();
    auto& stmt = arena.get(stmt_ref);
    pending_stmts.pop_back();

    if (auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
      AppendRange(pending_stmts,
                  std::ranges::reverse_view(if_stmt->else_body.statements));
      AppendRange(pending_stmts,
                  std::ranges::reverse_view(if_stmt->then_body.statements));

      expr_from_type[if_stmt->cond] = "Bool";

      pending_exprs.push_back(if_stmt->cond);
    } else if (auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
      const auto& result_type = std::get<BasicType>(
          std::get<Type>(std::get<Expr>(arena.get(func_def.result_type))));
      expr_from_type[return_stmt->value] = result_type.name;

      pending_exprs.push_back(return_stmt->value);
    } else if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
      ident_from_type[var_decl_stmt->name] = var_decl_stmt->type;
      expr_from_type[var_decl_stmt->init] = var_decl_stmt->type;

      pending_exprs.push_back(var_decl_stmt->init);
    } else if (std::holds_alternative<Expr>(stmt)) {
      pending_exprs.push_back(stmt_ref);
    }

    while (!pending_exprs.empty()) {
      auto expr_ref = pending_exprs.back();
      auto& expr = std::get<Expr>(arena.get(expr_ref));
      pending_exprs.pop_back();

      if (auto* func_call_expr = std::get_if<FuncCallExpr>(&expr)) {
        const auto& func_type = func_types.at(func_call_expr->func_name);
        for (int i = 0; i < func_call_expr->arguments.size(); ++i) {
          const auto& arg = func_call_expr->arguments[i];
          const auto& param_type = std::get<BasicType>(std::get<Type>(
              std::get<Expr>(arena.get(func_type.parameters[i].type))));
          expr_from_type[arg] = param_type.name;
          pending_exprs.push_back(arg);
        }
      } else if (std::holds_alternative<BoolLitExpr>(expr)) {
        if (auto it = expr_from_type.find(expr_ref);
            it != expr_from_type.end() && it->second != "Bool") {
          return TypeError(std::string("Bool literal is not of type ") +
                           std::string(it->second));
        }
        expr_from_type[expr_ref] = "Bool";
      } else if (auto* ident_expr = std::get_if<IdentExpr>(&expr)) {
        if (auto it = expr_from_type.find(expr_ref);
            it != expr_from_type.end() &&
            it->second != ident_from_type[ident_expr->name]) {
          return TypeError(
              std::string("Identifier '") + std::string(ident_expr->name) +
              std::string("' is not of type ") + std::string(it->second));
        }
        expr_from_type[expr_ref] = ident_from_type[ident_expr->name];
      } else if (auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
        if (binary_op_expr->op == BinaryOp::Eq) {
          expr_from_expr[binary_op_expr->rhs] = binary_op_expr->lhs;
          expr_from_expr[binary_op_expr->lhs] = binary_op_expr->rhs;
        } else {
          expr_from_expr[binary_op_expr->rhs] = binary_op_expr->lhs;
          expr_from_expr[expr_ref] = binary_op_expr->rhs;
          expr_from_expr[binary_op_expr->lhs] = expr_ref;
        }

        pending_exprs.push_back(binary_op_expr->rhs);
        pending_exprs.push_back(binary_op_expr->lhs);
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
    SetType(std::get<Expr>(arena.get(expr_ref)), type);
  }

  return std::nullopt;
}

}  // namespace lucid
