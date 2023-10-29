#pragma once

#include <cstddef>
#include <functional>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

template <typename T, typename P>
bool AllMatch(const std::vector<T>& real, const std::vector<P>& patterns) {
  if (real.size() != patterns.size()) return false;
  for (std::size_t i = 0; i < real.size(); ++i) {
    if (!patterns[i](real[i])) return false;
  }
  return true;
}

using StmtRefMatcher = std::function<bool(StmtRef)>;

using ExprRefMatcher = std::function<bool(ExprRef)>;

using TypeRefMatcher = std::function<bool(TypeRef)>;

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

struct ReturnStmtPattern {
  StmtRefMatcher value;

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

struct FuncCallExprPattern {
  TypeRefMatcher type;
  std::string_view func_name;
  std::vector<ExprRefMatcher> arguments;

  bool operator()(const FuncCallExpr& expr) const {
    if (type != nullptr && !type(expr.type)) return false;
    return func_name == expr.func_name && AllMatch(expr.arguments, arguments);
  }
};

struct VarDeclStmtPattern {
  TypeRefMatcher type;
  std::string_view name;
  ExprRefMatcher init;

  bool operator()(const VarDeclStmt& stmt) const {
    if (type != nullptr && !type(stmt.type)) return false;
    return name == stmt.name && init(stmt.init);
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

class AstFixture {
 protected:
  // *A*llocates the statement `stmt` on an *A*rena.
  template <typename T>
  StmtRef A(T stmt) {
    return arena_.add(stmt);
  }

  std::function<bool(FuncDefStmt)> MatchesFuncDefStmt(
      FuncDefStmtPattern pattern) {
    return [pattern](FuncDefStmt stmt) { return pattern(stmt); };
  }

  StmtRefMatcher MatchesReturnStmt(ReturnStmtPattern pattern) {
    return MatchesStmt<ReturnStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesIfStmt(IfStmtPattern pattern) {
    return MatchesStmt<IfStmt>(std::move(pattern));
  }

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

  Arena<Stmt> arena_;

 private:
  template <typename S, typename P>
  ExprRefMatcher MatchesStmt(P pattern) {
    return [this, pattern](ExprRef ref) {
      if (auto* stmt = std::get_if<S>(&arena_.get(ref))) return pattern(*stmt);
      return false;
    };
  }

  template <typename E, typename P>
  ExprRefMatcher MatchesExpr(P pattern) {
    return MatchesStmt<Expr>([pattern](const Expr& stmt) {
      if (auto* expr = std::get_if<E>(&stmt)) return pattern(*expr);
      return false;
    });
  }

  template <typename T, typename P>
  TypeRefMatcher MatchesType(P pattern) {
    return MatchesExpr<Type>([pattern](const Type& expr) {
      if (auto* type = std::get_if<T>(&expr)) return pattern(*type);
      return false;
    });
  }
};

}  // namespace lucid
