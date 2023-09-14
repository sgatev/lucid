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

std::optional<TypeError> InferExpressionTypes(Arena<Stmt>& arena,
                                              FuncDefStmt& func_def) {
  std::unordered_map<std::string_view, std::string_view> ident_types;
  for (const auto& param : func_def.parameters) {
    ident_types[param.name] = param.type;
  }

  std::vector<StmtRef> pending_stmts;
  for (auto stmt : std::ranges::reverse_view(func_def.body.statements)) {
    pending_stmts.push_back(stmt);
  }

  std::vector<std::pair<ExprRef, std::string_view>> pending_exprs;
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
      pending_exprs.emplace_back(cstmt->cond, "");
    } else if (auto* cstmt = std::get_if<ReturnStmt>(&stmt)) {
      pending_exprs.emplace_back(cstmt->value, func_def.result_type);
    } else if (auto* cstmt = std::get_if<VarDeclStmt>(&stmt)) {
      ident_types[cstmt->name] = cstmt->type;

      pending_exprs.emplace_back(cstmt->init, cstmt->type);
    }

    while (!pending_exprs.empty()) {
      auto [expr_ref, type] = pending_exprs.back();
      auto& expr = std::get<Expr>(arena.get(expr_ref));
      pending_exprs.pop_back();

      if (auto* cexpr = std::get_if<FuncCallExpr>(&expr)) {
        cexpr->type = type;
        for (const auto& arg : cexpr->arguments) {
          pending_exprs.emplace_back(arg, "");
        }
      } else if (auto* cexpr = std::get_if<IntLitExpr>(&expr)) {
        cexpr->type = type;
      } else if (auto* cexpr = std::get_if<BoolLitExpr>(&expr)) {
        if (type == "") {
          cexpr->type = "Bool";
          continue;
        }
        if (type != "Bool") {
          return TypeError(std::string("Bool literal is not of type ") +
                           std::string(type));
        }
        cexpr->type = type;
      } else if (auto* cexpr = std::get_if<IdentExpr>(&expr)) {
        if (type == "") {
          cexpr->type = ident_types[cexpr->name];
          continue;
        } else if (ident_types[cexpr->name] != type) {
          return TypeError(
              std::string("Identifier '") + std::string(cexpr->name) +
              std::string("' is not of type ") + std::string(type));
        }
        cexpr->type = type;
      } else if (auto* cexpr = std::get_if<BinaryOpExpr>(&expr)) {
        cexpr->type = type;
        pending_exprs.emplace_back(cexpr->rhs, type);
        pending_exprs.emplace_back(cexpr->lhs, type);
      }
    }
  }

  return std::nullopt;
}

}  // namespace lucid
