#include "lucid/type.h"

#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
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

class ExprTypeInferenceEngine {
 public:
  ExprTypeInferenceEngine(
      const Arena<Stmt>& stmt_arena, Arena<Expr>& expr_arena,
      Arena<Type>& type_arena,
      const std::unordered_map<std::string_view, FuncType>& func_types,
      FuncDefStmt& func_def)
      : stmt_arena_(stmt_arena),
        expr_arena_(expr_arena),
        type_arena_(type_arena),
        func_types_(func_types),
        func_def_(func_def) {}

  std::optional<TypeError> InferTypes() {
    for (const auto& param : func_def_.parameters) {
      SetIdentType(param.name, param.type);
    }

    AddPendingStmts(std::ranges::reverse_view(
        std::ranges::subrange(func_def_.stmts.begin(), func_def_.stmts.end())));

    while (true) {
      auto stmt_ref = NextStmt();
      if (!stmt_ref.has_value()) break;
      const auto& stmt = DerefStmt(*stmt_ref);

      ProcessPendingStmt(*stmt_ref, stmt);

      while (true) {
        auto expr_ref = NextExpr();
        if (!expr_ref.has_value()) break;
        const auto& expr = DerefExpr(*expr_ref);

        ProcessPendingExpr(*expr_ref, expr);
      }
    }

    if (auto error = GetError(); error.has_value()) {
      return TypeError(std::move(*error));
    }

    SolveTypeEquations();

    for (auto int_lit_expr : int_lit_exprs_) {
      if (expr_from_type_.find(int_lit_expr) == expr_from_type_.end()) {
        expr_from_type_[int_lit_expr] =
            type_arena_.add(BasicType{.name = "Int32"});
      }
    }

    for (auto [expr_ref, type] : expr_from_type_) {
      SetType(DerefExpr(expr_ref), type);
    }

    return std::nullopt;
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
    RequireTypeForExpr(stmt.cond, type_arena_.add(BasicType{.name = "Bool"}));
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
    SetIdentType(stmt.name, stmt.type);
    if (stmt.init.has_value()) {
      if (auto* array_type = std::get_if<ArrayType>(&DerefType(stmt.type))) {
        RequireTypeForExpr(*stmt.init, array_type->element_type);
      } else {
        RequireTypeForExpr(*stmt.init, stmt.type);
      }
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

  void ProcessPendingStmt(StmtRef stmt_ref, const Expr& stmt) {
    AddPendingExpr(stmt_ref);
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
    const auto& func_type = func_types_.at(expr.func_name);
    for (std::uint8_t i = 0; i < expr.args.size(); ++i) {
      ExprRef arg = expr.args[i];
      RequireTypeForExpr(arg, func_type.parameters[i].type);
      AddPendingExpr(arg);
    }
    RequireTypeForExpr(
        expr_ref, type_arena_.add(BasicType{.name = func_type.result_type}));
  }

  void ProcessPendingExpr(ExprRef expr_ref, const BoolLitExpr& expr) {
    RequireTypeForExpr(expr_ref, type_arena_.add(BasicType{.name = "Bool"}));
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
      RequireTypeForExpr(expr_ref, type_arena_.add(BasicType{.name = "Bool"}));
    } else {
      RequireSameTypesForExprs(expr.rhs, expr.lhs);
      RequireSameTypesForExprs(expr_ref, expr.rhs);
      RequireSameTypesForExprs(expr.lhs, expr_ref);
    }

    AddPendingExpr(expr.rhs);
    AddPendingExpr(expr.lhs);
  }

  void RequireTypeForExpr(ExprRef expr_ref, TypeRef type_ref) {
    if (auto it = expr_from_type_.find(expr_ref);
        it != expr_from_type_.end() && !TypesEqual(it->second, type_ref)) {
      if (std::holds_alternative<ArrayType>(DerefType(it->second))) {
        errors_.push_back(std::string("expected array type"));
      } else {
        const auto& it_type = std::get<BasicType>(DerefType(it->second));
        errors_.push_back(std::string("expected type ") +
                          std::string(it_type.name));
      }
    }
    expr_from_type_[expr_ref] = type_ref;
  }

  void RequireSameTypesForExprs(ExprRef lhs, ExprRef rhs) {
    expr_from_expr_[lhs] = rhs;
  }

  void RequireArrayElementTypeForExpr(ExprRef element, ExprRef array) {
    expr_from_array_[element] = array;
  }

  void SetIdentType(std::string_view name, TypeRef type_ref) {
    ident_from_type_[name] = type_ref;
  }

  TypeRef GetIdentType(std::string_view name) const {
    return ident_from_type_.at(name);
  }

  bool TypesEqual(TypeRef lhs_ref, TypeRef rhs_ref) {
    auto lhs_type = DerefType(lhs_ref);
    auto rhs_type = DerefType(rhs_ref);
    if (lhs_type.index() != rhs_type.index()) return false;

    if (std::holds_alternative<ArrayType>(lhs_type)) {
      const auto& lhs = std::get<ArrayType>(lhs_type);
      const auto& rhs = std::get<ArrayType>(rhs_type);
      return TypesEqual(lhs.element_type, rhs.element_type) &&
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

  const Stmt& DerefStmt(StmtRef ref) { return stmt_arena_.get(ref); }

  Expr& DerefExpr(ExprRef ref) { return expr_arena_.get(ref); }

  Type& DerefType(TypeRef ref) { return type_arena_.get(ref); }

  void SolveTypeEquations() {
    while (true) {
      for (auto [element, array] : expr_from_array_) {
        if (auto it = expr_from_type_.find(array);
            it != expr_from_type_.end()) {
          const auto& array_type = std::get<ArrayType>(DerefType(it->second));
          expr_from_type_[element] = array_type.element_type;
        }
      }

      std::unordered_map<ExprRef, ExprRef> next_expr_from_expr;
      for (auto [lhs, rhs] : expr_from_expr_) {
        if (auto it = expr_from_type_.find(rhs); it != expr_from_type_.end()) {
          expr_from_type_[lhs] = it->second;
        } else {
          next_expr_from_expr[lhs] = rhs;
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

  const Arena<Stmt>& stmt_arena_;
  Arena<Expr>& expr_arena_;
  Arena<Type>& type_arena_;
  const std::unordered_map<std::string_view, FuncType>& func_types_;
  FuncDefStmt& func_def_;

  std::unordered_map<ExprRef, ExprRef> expr_from_expr_;
  std::unordered_map<ExprRef, TypeRef> expr_from_type_;
  std::unordered_map<std::string_view, TypeRef> ident_from_type_;
  std::unordered_map<ExprRef, ExprRef> expr_from_array_;

  std::vector<StmtRef> pending_stmts_;
  std::vector<ExprRef> pending_exprs_;
  std::vector<ExprRef> int_lit_exprs_;

  std::vector<std::string> errors_;
};

}  // namespace

std::unordered_map<std::string_view, FuncType> ExtractFuncTypes(
    const Arena<Stmt>& stmt_arena, const Arena<Expr>& expr_arena,
    const Arena<Type>& type_arena, const std::vector<FuncDefStmt>& func_defs) {
  std::unordered_map<std::string_view, FuncType> func_types;
  for (const auto& func_def : func_defs) {
    const auto& result_type =
        std::get<BasicType>(type_arena.get(func_def.result_type));
    func_types[func_def.name] = {
        .result_type = result_type.name,
        .parameters = func_def.parameters,
    };
  }
  return func_types;
}

std::optional<TypeError> InferExprTypes(
    const Arena<Stmt>& stmt_arena, Arena<Expr>& expr_arena,
    Arena<Type>& type_arena,
    const std::unordered_map<std::string_view, FuncType>& func_types,
    FuncDefStmt& func_def) {
  return ExprTypeInferenceEngine(stmt_arena, expr_arena, type_arena, func_types,
                                 func_def)
      .InferTypes();
}

}  // namespace lucid
