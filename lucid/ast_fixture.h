#pragma once

#include <cstddef>
#include <functional>
#include <iterator>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "gmock/gmock.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/string_index.h"
#include "lucid/successive_list.h"

namespace lucid {

using StmtRefMatcher = std::function<bool(StmtRef)>;

using ExprRefMatcher = std::function<bool(ExprRef)>;

using TypeRefMatcher = std::function<bool(TypeRef)>;

using ParamRefMatcher = std::function<bool(ParamRef)>;

template <typename T, typename P>
bool AllMatch(const std::vector<T>& real, const std::vector<P>& patterns) {
  if (real.size() != patterns.size()) return false;
  for (std::size_t i = 0; i < real.size(); ++i) {
    if (!patterns[i](real[i])) return false;
  }
  return true;
}

template <typename T, typename P>
inline bool AllMatch(SuccessiveList<T> real, const std::vector<P>& patterns) {
  if (real.size() != patterns.size()) return false;
  for (int i = 0; i < real.size(); ++i) {
    if (!patterns[i](real[i])) return false;
  }
  return true;
}

struct FuncParamPattern {
  StringIndex::Ref name;
  TypeRefMatcher type_constraint;

  bool operator()(const FuncParam& param) const {
    if (type_constraint != nullptr && !type_constraint(param.type_constraint)) {
      return false;
    }
    return name == param.name;
  }
};

struct CompoundStmtPattern {
  std::vector<StmtRefMatcher> statements;
};

struct FuncDefStmtPattern {
  StringIndex::Ref name;
  std::vector<ParamRefMatcher> params;
  TypeRefMatcher result_type;
  CompoundStmtPattern body;

  bool operator()(const FuncDefStmt& stmt) const {
    if (result_type != nullptr && !result_type(stmt.result_type)) return false;
    return name == stmt.name && AllMatch(stmt.params, params) &&
           AllMatch(stmt.stmts, body.statements);
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
    return cond(stmt.cond) && AllMatch(stmt.then_stmts, then_body.statements) &&
           AllMatch(stmt.else_stmts, else_body.statements);
  }
};

struct LoopStmtPattern {
  CompoundStmtPattern body;

  bool operator()(const LoopStmt& stmt) const {
    return AllMatch(stmt.stmts, body.statements);
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
  TypeRefMatcher type_constraint;
  std::string_view name;
  ExprRefMatcher init;

  bool operator()(const VarDeclStmt& stmt) const {
    if (type_constraint != nullptr && !type_constraint(stmt.type_constraint)) {
      return false;
    }
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
  TypeRefMatcher element_type_constraint;
  IntLitExprPattern size;

  bool operator()(const ArrayType& type) const {
    return element_type_constraint(type.element_type_constraint) &&
           size(type.size);
  }
};

class AstFixture {
 protected:
  // Allocates the statement `stmt` on an arena.
  template <typename X>
  StmtRef S(X stmt) {
    return ctx_.Add(stmt);
  }

  // Allocates the expression `expr` on an arena.
  template <typename X>
  ExprRef E(X expr) {
    return ctx_.Add(expr);
  }

  // Allocates the type `type` on an arena.
  template <typename X>
  TypeRef T(X type) {
    return ctx_.Add(type);
  }

  // Allocates the type `param` on an arena.
  template <typename X>
  ParamRef P(X param) {
    return ctx_.Add(param);
  }

  // Allocates the `ident`.
  StringIndex::Ref I(std::string_view ident) { return ctx_.AddIdent(ident); }

  // Returns an empty list.
  template <typename T>
  SuccessiveList<typename Arena<T>::Ref> EmptyList() {
    return SuccessiveList<typename Arena<T>::Ref>(0, Arena<T>::kNullRef);
  }

  // Creates a list of the given expression references.
  SuccessiveList<ExprRef> ExprListOf(std::initializer_list<ExprRef> exprs) {
    if (std::empty(exprs)) return EmptyList<Expr>();
    auto it = exprs.begin();
    auto first_expr = ctx_.AliasExpr(*it);
    ++it;
    for (; it != exprs.end(); ++it) ctx_.AliasExpr(*it);
    return SuccessiveList<ExprRef>(exprs.size(), first_expr);
  }

  // Creates a list of the given statement references.
  SuccessiveList<StmtRef> StmtListOf(std::initializer_list<StmtRef> stmts) {
    if (std::empty(stmts)) return EmptyList<Stmt>();
    auto it = stmts.begin();
    auto first_stmt = ctx_.AliasStmt(*it);
    ++it;
    for (; it != stmts.end(); ++it) ctx_.AliasStmt(*it);
    return SuccessiveList<StmtRef>(stmts.size(), first_stmt);
  }

  // Creates a list of the given parameter references.
  SuccessiveList<ParamRef> ParamListOf(std::initializer_list<ParamRef> params) {
    if (std::empty(params)) return EmptyList<FuncParam>();
    auto it = params.begin();
    auto first_stmt = ctx_.AliasParam(*it);
    ++it;
    for (; it != params.end(); ++it) ctx_.AliasParam(*it);
    return SuccessiveList<ParamRef>(params.size(), first_stmt);
  }

  // Returns a matcher that is satisfied if the argument is a statement
  // reference equivalent to `expected`.
  auto StmtEquivTo(StmtRef expected) {
    return testing::Truly([this, expected](StmtRef actual) {
      return ctx_.EquivStmts(expected, actual);
    });
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

  ParamRefMatcher MatchesFuncParam(FuncParamPattern pattern) {
    return
        [this, pattern](ParamRef ref) { return pattern(ctx_.DerefParam(ref)); };
  }

  SyntaxContext ctx_;

 private:
  template <typename S>
  StmtRefMatcher MatchesStmt() {
    return [this](StmtRef ref) {
      return std::holds_alternative<S>(ctx_.DerefStmt(ref));
    };
  }

  template <typename S, typename P>
  StmtRefMatcher MatchesStmt(P pattern) {
    return [this, pattern](StmtRef ref) {
      if (auto* stmt = std::get_if<S>(&ctx_.DerefStmt(ref))) {
        return pattern(*stmt);
      }
      return false;
    };
  }

  template <typename E, typename P>
  ExprRefMatcher MatchesExpr(P pattern) {
    return [this, pattern](ExprRef ref) {
      if (auto* expr = std::get_if<E>(&ctx_.DerefExpr(ref))) {
        return pattern(*expr);
      }
      return false;
    };
  }

  template <typename T, typename P>
  TypeRefMatcher MatchesType(P pattern) {
    return [this, pattern](TypeRef ref) {
      if (auto* type = std::get_if<T>(&ctx_.DerefType(ref))) {
        return pattern(*type);
      }
      return false;
    };
  }
};

}  // namespace lucid
