#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

using StmtRefMatcher = std::function<bool(StmtRef)>;

using ExprRefMatcher = std::function<bool(ExprRef)>;

using TypeRefMatcher = std::function<bool(TypeRef)>;

template <typename T, typename P>
bool AllMatch(const std::vector<T>& real, const std::vector<P>& patterns) {
  if (real.size() != patterns.size()) return false;
  for (std::size_t i = 0; i < real.size(); ++i) {
    if (!patterns[i](real[i])) return false;
  }
  return true;
}

inline bool AllMatch(ExprList real,
                     const std::vector<ExprRefMatcher>& patterns) {
  if (real.size() != patterns.size()) return false;
  for (int i = 0; i < real.size(); ++i) {
    if (!patterns[i](real[i])) return false;
  }
  return true;
}

struct FuncParamPattern {
  std::string_view name;
  TypeRefMatcher type;

  bool operator()(const FuncParam& param) const {
    if (type != nullptr && !type(param.type)) return false;
    return name == param.name;
  }
};

struct CompoundStmtPattern {
  std::vector<StmtRefMatcher> statements;
};

struct FuncDefStmtPattern {
  std::string_view name;
  std::vector<FuncParamPattern> parameters;
  TypeRefMatcher result_type;
  CompoundStmtPattern body;

  bool operator()(const FuncDefStmt& stmt) const {
    if (result_type != nullptr && !result_type(stmt.result_type)) return false;
    return name == stmt.name && AllMatch(stmt.parameters, parameters) &&
           AllMatch(stmt.body.statements, body.statements);
  }
};

struct DoStmtPattern {
  ExprRefMatcher expr;

  bool operator()(const DoStmt& stmt) const { return expr(stmt.expr); }
};

struct ReturnStmtPattern {
  ExprRefMatcher value;

  bool operator()(const ReturnStmt& stmt) const { return value(stmt.value); }
};

struct IfStmtPattern {
  ExprRefMatcher cond;
  CompoundStmtPattern then_body;
  CompoundStmtPattern else_body;

  bool operator()(const IfStmt& stmt) const {
    return cond(stmt.cond) &&
           AllMatch(stmt.then_body.statements, then_body.statements) &&
           AllMatch(stmt.else_body.statements, else_body.statements);
  }
};

struct LoopStmtPattern {
  CompoundStmtPattern body;

  bool operator()(const LoopStmt& stmt) const {
    return AllMatch(stmt.body.statements, body.statements);
  }
};

struct IntLitExprPattern {
  TypeRefMatcher type;
  std::string_view value;

  bool operator()(const IntLitExpr& expr) const {
    if (type != nullptr && !type(expr.type)) return false;
    return value == expr.value;
  }
};

struct BoolLitExprPattern {
  std::string_view value;

  bool operator()(const BoolLitExpr& expr) const { return value == expr.value; }
};

struct StringLitExprPattern {
  TypeRefMatcher type;
  std::string_view value;

  bool operator()(const StringLitExpr& expr) const {
    if (type != nullptr && !type(expr.type)) return false;
    return value == expr.value;
  }
};

struct BinaryOpExprPattern {
  TypeRefMatcher type;
  BinaryOp op;
  ExprRefMatcher lhs;
  ExprRefMatcher rhs;

  bool operator()(const BinaryOpExpr& expr) const {
    if (type != nullptr && !type(expr.type)) return false;
    return op == expr.op && lhs(expr.lhs) && rhs(expr.rhs);
  }
};

struct IdentExprPattern {
  TypeRefMatcher type;
  std::string_view name;

  bool operator()(const IdentExpr& expr) const {
    if (type != nullptr && !type(expr.type)) return false;
    return name == expr.name;
  }
};

struct IndexExprPattern {
  TypeRefMatcher type;
  ExprRefMatcher base;
  ExprRefMatcher index;

  bool operator()(const IndexExpr& expr) const {
    if (type != nullptr && !type(expr.type)) return false;
    return base(expr.base) && index(expr.index);
  }
};

struct FuncCallExprPattern {
  TypeRefMatcher type;
  std::string_view func_name;
  std::vector<ExprRefMatcher> args;

  bool operator()(const FuncCallExpr& expr) const {
    if (type != nullptr && !type(expr.type)) return false;
    return func_name == expr.func_name && AllMatch(expr.args, args);
  }
};

struct VarDeclStmtPattern {
  TypeRefMatcher type;
  std::string_view name;
  ExprRefMatcher init;

  bool operator()(const VarDeclStmt& stmt) const {
    if (type != nullptr && !type(stmt.type)) return false;
    if (init != nullptr) {
      if (!stmt.init.has_value()) return false;
      if (init != nullptr && !init(*stmt.init)) return false;
    }
    return name == stmt.name;
  }
};

struct VarAssignStmtPattern {
  std::string_view name;
  ExprRefMatcher expr;

  bool operator()(const VarAssignStmt& stmt) const {
    return name == stmt.name && expr(stmt.expr);
  }
};

struct BasicTypePattern {
  std::string_view name;

  bool operator()(const BasicType& type) const { return name == type.name; }
};

struct ArrayTypePattern {
  TypeRefMatcher element_type;
  IntLitExprPattern size;

  bool operator()(const ArrayType& type) const {
    return element_type(type.element_type) && size(type.size);
  }
};

class AstFixture {
 protected:
  // Allocates the statement `stmt` on an arena.
  template <typename S>
  StmtRef S(S stmt) {
    return stmt_arena_.add(stmt);
  }

  // Allocates the expression `expr` on an arena.
  template <typename E>
  ExprRef E(E expr) {
    return expr_arena_.add(expr);
  }

  // Allocates the type `type` on an arena.
  template <typename T>
  TypeRef T(T type) {
    return type_arena_.add(type);
  }

  // Returns an empty expression list.
  ExprList EmptyExprList() { return ExprList(0, Arena<Expr>::kNullRef); }

  // Allocates the given expressions on an arena and returns a list of
  // references.
  template <typename E, typename... Es>
  ExprList ExprListOf(E expr, Es... exprs) {
    auto first_expr = expr_arena_.add(expr);
    (expr_arena_.add(exprs), ...);
    return ExprList(1 + sizeof...(Es), first_expr);
  }

  ExprRefMatcher MatchesAnyExpr() {
    return [](ExprRef) { return true; };
  }

  std::function<bool(FuncDefStmt)> MatchesFuncDefStmt(
      FuncDefStmtPattern pattern) {
    return [pattern](FuncDefStmt stmt) { return pattern(stmt); };
  }

  StmtRefMatcher MatchesDoStmt(DoStmtPattern pattern) {
    return MatchesStmt<DoStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesReturnStmt(ReturnStmtPattern pattern) {
    return MatchesStmt<ReturnStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesIfStmt(IfStmtPattern pattern) {
    return MatchesStmt<IfStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesLoopStmt(LoopStmtPattern pattern) {
    return MatchesStmt<LoopStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesBreakStmt() { return MatchesStmt<BreakStmt>(); }

  ExprRefMatcher MatchesIntLitExpr(IntLitExprPattern pattern) {
    return MatchesExpr<IntLitExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesBoolLitExpr(BoolLitExprPattern pattern) {
    return MatchesExpr<BoolLitExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesStringLitExpr(StringLitExprPattern pattern) {
    return MatchesExpr<StringLitExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesBinaryOpExpr(BinaryOpExprPattern pattern) {
    return MatchesExpr<BinaryOpExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesIdentExpr(IdentExprPattern pattern) {
    return MatchesExpr<IdentExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesIndexExpr(IndexExprPattern pattern) {
    return MatchesExpr<IndexExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesFuncCallExpr(FuncCallExprPattern pattern) {
    return MatchesExpr<FuncCallExpr>(std::move(pattern));
  }

  StmtRefMatcher MatchesVarDeclStmt(VarDeclStmtPattern pattern) {
    return MatchesStmt<VarDeclStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesVarAssignStmt(VarAssignStmtPattern pattern) {
    return MatchesStmt<VarAssignStmt>(std::move(pattern));
  }

  TypeRefMatcher MatchesBasicType(BasicTypePattern pattern) {
    return MatchesType<BasicType>(std::move(pattern));
  }

  TypeRefMatcher MatchesArrayType(ArrayTypePattern pattern) {
    return MatchesType<ArrayType>(std::move(pattern));
  }

  Arena<Stmt> stmt_arena_;
  Arena<Expr> expr_arena_;
  Arena<Type> type_arena_;

 private:
  template <typename S>
  ExprRefMatcher MatchesStmt() {
    return [this](StmtRef ref) {
      return std::holds_alternative<S>(stmt_arena_.get(ref));
    };
  }

  template <typename S, typename P>
  ExprRefMatcher MatchesStmt(P pattern) {
    return [this, pattern](StmtRef ref) {
      if (auto* stmt = std::get_if<S>(&stmt_arena_.get(ref))) {
        return pattern(*stmt);
      }
      return false;
    };
  }

  template <typename E, typename P>
  ExprRefMatcher MatchesExpr(P pattern) {
    return [this, pattern](ExprRef ref) {
      if (auto* expr = std::get_if<E>(&expr_arena_.get(ref))) {
        return pattern(*expr);
      }
      return false;
    };
  }

  template <typename T, typename P>
  TypeRefMatcher MatchesType(P pattern) {
    return [this, pattern](TypeRef ref) {
      if (auto* type = std::get_if<T>(&type_arena_.get(ref))) {
        return pattern(*type);
      }
      return false;
    };
  }
};

}  // namespace lucid
