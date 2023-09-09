#pragma once

#include <cstddef>
#include <functional>
#include <string_view>
#include <vector>

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

struct FuncParamPattern {
  std::string_view name;
  std::string_view type;

  bool operator()(const FuncParam& param) const {
    return name == param.name && type == param.type;
  }
};

struct CompoundStmtPattern {
  std::vector<StmtRefMatcher> statements;
};

struct FuncDefStmtPattern {
  std::string_view name;
  std::vector<FuncParamPattern> parameters;
  std::string_view result_type;
  CompoundStmtPattern body;

  bool operator()(const FuncDefStmt& stmt) const {
    return name == stmt.name && AllMatch(stmt.parameters, parameters) &&
           result_type == stmt.result_type &&
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
  std::string_view type;
  std::string_view value;

  bool operator()(const IntLitExpr& expr) const {
    return type == expr.type && value == expr.value;
  }
};

struct BoolLitExprPattern {
  std::string_view value;

  bool operator()(const BoolLitExpr& expr) const { return value == expr.value; }
};

struct BinaryOpExprPattern {
  std::string_view type;
  BinaryOp op;
  ExprRefMatcher lhs;
  ExprRefMatcher rhs;

  bool operator()(const BinaryOpExpr& expr) const {
    return type == expr.type && op == expr.op && lhs(expr.lhs) && rhs(expr.rhs);
  }
};

struct IdentExprPattern {
  std::string_view name;

  bool operator()(const IdentExpr& expr) const { return name == expr.name; }
};

struct FuncCallExprPattern {
  std::string_view func_name;
  std::vector<ExprRefMatcher> arguments;

  bool operator()(const FuncCallExpr& expr) const {
    return func_name == expr.func_name && AllMatch(expr.arguments, arguments);
  }
};

struct VarDeclStmtPattern {
  std::string_view type;
  std::string_view name;
  ExprRefMatcher init;

  bool operator()(const VarDeclStmt& stmt) const {
    return type == stmt.type && name == stmt.name && init(stmt.init);
  }
};

}  // namespace lucid
