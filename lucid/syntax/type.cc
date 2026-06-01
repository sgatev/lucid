#include "lucid/syntax/type.h"

#include <cstdint>
#include <expected>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

class ExprTypeInferenceEngine {
 public:
  ExprTypeInferenceEngine(SyntaxContext& syn_ctx,
                          const std::vector<FuncDefStmt>& func_defs,
                          FuncDefStmt& func_def)
      : syn_ctx_(syn_ctx), func_def_(func_def) {
    for (const auto& func : func_defs) func_defs_.Set(func.name, &func);
  }

  std::expected<void, TypeError> InferTypes() {
    for (const auto& param_ref : func_def_.params) {
      const auto& param = syn_ctx_.DerefParam(param_ref);
      SetIdentType(param.name, param.type_constraint);
    }

    AddPendingStmts(std::ranges::reverse_view(func_def_.stmts));

    while (true) {
      auto stmt_ref = NextStmt();
      if (!stmt_ref.has_value()) break;
      const auto& stmt = syn_ctx_.DerefStmt(*stmt_ref);

      ProcessPendingStmt(*stmt_ref, stmt);

      while (true) {
        auto expr_ref = NextExpr();
        if (!expr_ref.has_value()) break;
        const auto& expr = syn_ctx_.DerefExpr(*expr_ref);

        ProcessPendingExpr(*expr_ref, expr);
      }
    }

    if (auto error = GetError(); error.has_value()) {
      return std::unexpected(TypeError(std::move(*error)));
    }

    SolveTypeEquations();

    for (auto int_lit_expr : int_lit_exprs_) {
      if (!expr_from_type_.Get(int_lit_expr).has_value()) {
        expr_from_type_.Set(int_lit_expr,
                            ResolveType(syn_ctx_.AddIdent("Int32")));
      }
    }

    for (auto [expr_ref, type] : expr_from_type_) {
      SetType(syn_ctx_.DerefExpr(expr_ref), type);
    }

    return {};
  }

 private:
  void ProcessPendingStmt(StmtRef stmt_ref, const Stmt& stmt) {
    if (auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
      ProcessPendingStmt(stmt_ref, *loop_stmt);
    } else if (auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
      ProcessPendingStmt(stmt_ref, *if_stmt);
    } else if (auto* do_stmt = std::get_if<DoStmt>(&stmt)) {
      ProcessPendingStmt(stmt_ref, *do_stmt);
    } else if (auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
      ProcessPendingStmt(stmt_ref, *return_stmt);
    } else if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
      ProcessPendingStmt(stmt_ref, *var_decl_stmt);
    } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
      ProcessPendingStmt(stmt_ref, *var_assign_stmt);
    } else if (auto* array_assign_stmt = std::get_if<ArrayAssignStmt>(&stmt)) {
      ProcessPendingStmt(stmt_ref, *array_assign_stmt);
    }
  }

  void ProcessPendingStmt(StmtRef stmt_ref, const LoopStmt& stmt) {
    AddPendingStmts(std::ranges::reverse_view(stmt.stmts));
  }

  void ProcessPendingStmt(StmtRef stmt_ref, const IfStmt& stmt) {
    AddPendingStmts(std::ranges::reverse_view(stmt.else_stmts));
    AddPendingStmts(std::ranges::reverse_view(stmt.then_stmts));
    RequireTypeForExpr(stmt.cond, ResolveType(syn_ctx_.AddIdent("Bool")));
    AddPendingExpr(stmt.cond);
  }

  void ProcessPendingStmt(StmtRef stmt_ref, const DoStmt& stmt) {
    AddPendingExpr(stmt.expr);
  }

  void ProcessPendingStmt(StmtRef stmt_ref, const ReturnStmt& stmt) {
    RequireTypeForExpr(stmt.value, func_def_.result_type);
    AddPendingExpr(stmt.value);
  }

  void ProcessPendingStmt(StmtRef stmt_ref, const VarDeclStmt& stmt) {
    SetIdentType(stmt.name, stmt.type_constraint);
    if (stmt.init.has_value()) {
      RequireTypeForExpr(*stmt.init, stmt.type_constraint);
      AddPendingExpr(*stmt.init);
    }
  }

  void ProcessPendingStmt(StmtRef stmt_ref, const VarAssignStmt& stmt) {
    RequireTypeForExpr(stmt.expr, GetIdentType(stmt.name));
    AddPendingExpr(stmt.expr);
  }

  void ProcessPendingStmt(StmtRef stmt_ref, const ArrayAssignStmt& stmt) {
    AddPendingExpr(stmt.index);
    AddPendingExpr(stmt.expr);
  }

  void ProcessPendingExpr(ExprRef expr_ref, const Expr& expr) {
    if (auto* func_call_expr = std::get_if<FuncCallExpr>(&expr)) {
      ProcessPendingExpr(expr_ref, *func_call_expr);
    } else if (auto* bool_lit_expr = std::get_if<BoolLitExpr>(&expr)) {
      ProcessPendingExpr(expr_ref, *bool_lit_expr);
    } else if (auto* int_lit_expr = std::get_if<IntLitExpr>(&expr)) {
      ProcessPendingExpr(expr_ref, *int_lit_expr);
    } else if (auto* ident_expr = std::get_if<IdentExpr>(&expr)) {
      ProcessPendingExpr(expr_ref, *ident_expr);
    } else if (auto* index_expr = std::get_if<IndexExpr>(&expr)) {
      ProcessPendingExpr(expr_ref, *index_expr);
    } else if (auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
      ProcessPendingExpr(expr_ref, *binary_op_expr);
    }
  }

  void ProcessPendingExpr(ExprRef expr_ref, const FuncCallExpr& expr) {
    const auto& func_def = *func_defs_.Get(expr.func_name);
    for (std::uint32_t i = 0; i < expr.args.size(); ++i) {
      const auto& param = syn_ctx_.DerefParam(func_def->params[i]);
      ExprRef arg = expr.args[i];
      RequireTypeForExpr(arg, param.type_constraint);
      AddPendingExpr(arg);
    }
    RequireTypeForExpr(expr_ref, func_def->result_type);
  }

  void ProcessPendingExpr(ExprRef expr_ref, const BoolLitExpr& expr) {
    RequireTypeForExpr(expr_ref, ResolveType(syn_ctx_.AddIdent("Bool")));
  }

  void ProcessPendingExpr(ExprRef expr_ref, const IntLitExpr& expr) {
    int_lit_exprs_.push_back(expr_ref);
  }

  void ProcessPendingExpr(ExprRef expr_ref, const IdentExpr& expr) {
    RequireTypeForExpr(expr_ref, GetIdentType(expr.name));
  }

  void ProcessPendingExpr(ExprRef expr_ref, const IndexExpr& expr) {
    AddPendingExpr(expr.base);
    RequireArrayElementTypeForExpr(expr_ref, expr.base);
    AddPendingExpr(expr.index);
  }

  void ProcessPendingExpr(ExprRef expr_ref, const BinaryOpExpr& expr) {
    if (expr.op == BinaryOp::Eq || expr.op == BinaryOp::Lt ||
        expr.op == BinaryOp::Gt || expr.op == BinaryOp::NotEq) {
      RequireSameTypesForExprs(expr.rhs, expr.lhs);
      RequireSameTypesForExprs(expr.lhs, expr.rhs);
      RequireTypeForExpr(expr_ref, ResolveType(syn_ctx_.AddIdent("Bool")));
    } else {
      RequireSameTypesForExprs(expr.rhs, expr.lhs);
      RequireSameTypesForExprs(expr_ref, expr.rhs);
      RequireSameTypesForExprs(expr.lhs, expr_ref);
    }

    AddPendingExpr(expr.rhs);
    AddPendingExpr(expr.lhs);
  }

  void RequireTypeForExpr(ExprRef expr_ref, TypeRef type_ref) {
    if (auto it = expr_from_type_.Get(expr_ref);
        it.has_value() && !TypesEqual(*it, type_ref)) {
      if (std::holds_alternative<ArrayType>(syn_ctx_.DerefType(*it))) {
        errors_.push_back(std::string("expected array type"));
      } else {
        const auto& it_type = std::get<BasicType>(syn_ctx_.DerefType(*it));
        errors_.push_back(std::string("expected type ") +
                          std::string(syn_ctx_.DerefIdent(it_type.name)));
      }
    }
    expr_from_type_.Set(expr_ref, type_ref);
  }

  void RequireSameTypesForExprs(ExprRef lhs, ExprRef rhs) {
    expr_from_expr_.Set(lhs, rhs);
  }

  void RequireArrayElementTypeForExpr(ExprRef element, ExprRef array) {
    expr_from_array_.Set(element, array);
  }

  void SetIdentType(StringIndex::Ref ident, TypeRef type_ref) {
    ident_from_type_.Set(ident, type_ref);
  }

  TypeRef GetIdentType(StringIndex::Ref name) const {
    return *ident_from_type_.Get(name);
  }

  bool TypesEqual(TypeRef lhs_ref, TypeRef rhs_ref) {
    auto lhs_type = syn_ctx_.DerefType(lhs_ref);
    auto rhs_type = syn_ctx_.DerefType(rhs_ref);
    if (lhs_type.index() != rhs_type.index()) return false;

    if (std::holds_alternative<ArrayType>(lhs_type)) {
      const auto& lhs = std::get<ArrayType>(lhs_type);
      const auto& rhs = std::get<ArrayType>(rhs_type);
      return TypesEqual(lhs.element_type_constraint,
                        rhs.element_type_constraint) &&
             lhs.size.value == rhs.size.value;
    }

    const auto& lhs = std::get<BasicType>(lhs_type);
    const auto& rhs = std::get<BasicType>(rhs_type);
    return lhs.name == rhs.name;
  }

  std::optional<StmtRef> NextStmt() {
    if (pending_stmts_.empty()) return std::nullopt;
    auto stmt_ref = pending_stmts_.back();
    pending_stmts_.pop_back();
    return stmt_ref;
  }

  std::optional<ExprRef> NextExpr() {
    if (pending_exprs_.empty()) return std::nullopt;
    auto expr_ref = pending_exprs_.back();
    pending_exprs_.pop_back();
    return expr_ref;
  }

  template <std::ranges::view V>
  void AddPendingStmts(V stmts) {
    for (auto stmt : stmts) pending_stmts_.push_back(stmt);
  }

  void AddPendingExpr(ExprRef expr_ref) { pending_exprs_.push_back(expr_ref); }

  void SolveTypeEquations() {
    while (true) {
      for (auto [element, array] : expr_from_array_) {
        if (auto it = expr_from_type_.Get(array); it.has_value()) {
          const auto& array_type = std::get<ArrayType>(syn_ctx_.DerefType(*it));
          expr_from_type_.Set(element, array_type.element_type_constraint);
        }
      }

      HashMap<ExprRef, ExprRef> next_expr_from_expr;
      for (auto [lhs, rhs] : expr_from_expr_) {
        if (auto it = expr_from_type_.Get(rhs); it.has_value()) {
          expr_from_type_.Set(lhs, *it);
        } else {
          next_expr_from_expr.Set(lhs, rhs);
        }
      }
      if (next_expr_from_expr.size() == expr_from_expr_.size()) break;
      expr_from_expr_ = std::move(next_expr_from_expr);
    }
  }

  std::optional<std::string> GetError() {
    if (errors_.empty()) return std::nullopt;
    return std::move(errors_[0]);
  }

  TypeRef ResolveType(StringIndex::Ref name_ref) {
    std::string_view name = syn_ctx_.DerefIdent(name_ref);
    if (name == "Int32") {
      return syn_ctx_.Add(BasicType{
          .name = name_ref,
          .size = 4,
      });
    }
    if (name == "Int64") {
      return syn_ctx_.Add(BasicType{
          .name = name_ref,
          .size = 8,
      });
    }
    if (name == "Bool") {
      return syn_ctx_.Add(BasicType{
          .name = name_ref,
          .size = 4,
      });
    }
    if (name == "Void") {
      return syn_ctx_.Add(BasicType{
          .name = name_ref,
          .size = 0,
      });
    }
    if (name == "Double") {
      return syn_ctx_.Add(BasicType{
          .name = name_ref,
          .size = 8,
      });
    }
    if (name == "String") {
      return syn_ctx_.Add(BasicType{
          .name = name_ref,
          .size = 8,
      });
    }
    assert(false);
  }

  SyntaxContext& syn_ctx_;
  HashMap<StringIndex::Ref, const FuncDefStmt*> func_defs_;
  FuncDefStmt& func_def_;

  HashMap<ExprRef, ExprRef> expr_from_expr_;
  HashMap<ExprRef, TypeRef> expr_from_type_;
  HashMap<StringIndex::Ref, TypeRef> ident_from_type_;
  HashMap<ExprRef, ExprRef> expr_from_array_;

  std::vector<StmtRef> pending_stmts_;
  std::vector<ExprRef> pending_exprs_;
  std::vector<ExprRef> int_lit_exprs_;

  std::vector<std::string> errors_;
};

}  // namespace

std::expected<void, TypeError> InferExprTypes(
    SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    FuncDefStmt& func_def) {
  return ExprTypeInferenceEngine(syn_ctx, func_defs, func_def).InferTypes();
}

}  // namespace lucid
