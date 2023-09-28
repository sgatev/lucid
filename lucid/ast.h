#pragma once

#include <string_view>
#include <variant>
#include <vector>

#include "lucid/arena.h"

namespace lucid {

struct CompoundStmt;
struct FuncDefStmt;
struct ReturnStmt;
struct IntLitExpr;
struct BoolLitExpr;
struct FuncCallExpr;
struct VarDeclStmt;
struct IdentExpr;
struct BinaryOpExpr;
struct IfStmt;

// An expression in the Lucid language.
using Expr = std::variant<FuncCallExpr, IntLitExpr, BoolLitExpr, IdentExpr,
                          BinaryOpExpr>;

// A statement in the Lucid language.
using Stmt = std::variant<Expr, VarDeclStmt, FuncDefStmt, ReturnStmt, IfStmt>;

// A reference to a statement that can be dereferenced using an `Arena<Stmt>`
// object.
using StmtRef = ArenaRef<Stmt>;

// A reference to an expression that can be dereferenced using an `Arena<Stmt>`
// object.
using ExprRef = StmtRef;

// A list of zero or more statements.
struct CompoundStmt {
  // Statements in the list.
  std::vector<StmtRef> statements;
};

// A function parameter.
struct FuncParam {
  // Type of the parameter.
  std::string_view type;

  // Name of the parameter.
  std::string_view name;

  bool operator==(const FuncParam&) const = default;
};

// A statement that represents a function definition.
struct FuncDefStmt {
  // Name of the function.
  std::string_view name;

  // Body of the function.
  CompoundStmt body;

  // Type of the result of the function.
  std::string_view result_type;

  // Parameters of the function.
  std::vector<FuncParam> parameters;
};

// A statement that represents a return point in a function.
struct ReturnStmt {
  // Value that is returned by the function.
  ExprRef value;
};

// An expression that represents an integer literal.
struct IntLitExpr {
  // Type of the expression.
  std::string_view type;

  // Value of the integer.
  std::string_view value;
};

// An expression that represents a boolean literal.
struct BoolLitExpr {
  // Type of the expression.
  std::string_view type;

  // Value of the boolean.
  std::string_view value;
};

// An expression that represents a function call.
struct FuncCallExpr {
  // Type of the expression.
  std::string_view type;

  // Name of the function.
  std::string_view func_name;

  // Arguments to the function call.
  std::vector<ExprRef> arguments;
};

// A statement that represents a variable declaration.
struct VarDeclStmt {
  // Type of the variable.
  std::string_view type;

  // Name of the variable.
  std::string_view name;

  // Initializer expression.
  ExprRef init;
};

// An expression that represents an identifier.
struct IdentExpr {
  // Type of the expression.
  std::string_view type;

  // Name of the identifier.
  std::string_view name;
};

// A binary operation kind.
enum class BinaryOp {
  // Binary addition operation.
  Add,

  // Binary subtraction operation.
  Sub,

  // Binary multiplication operation.
  Mul,

  // Binary division operation.
  Div,

  // Greater than comparison operation.
  Gt,

  // Less than comparison operation.
  Lt,

  // Equals comparison operator.
  Eq,
};

// An expression that represents a binary operation over the values of two
// sub-expressions.
struct BinaryOpExpr {
  // Type of the expression.
  std::string_view type;

  // Binary operation kind.
  BinaryOp op;

  // Left-hand side sub-expression.
  ExprRef lhs;

  // Right-hand side sub-expression.
  ExprRef rhs;
};

// A statement that represents conditional execution.
struct IfStmt {
  // Condition that determines which branch of the statement will execute.
  ExprRef cond;

  // Body of the branch where the condition is true.
  CompoundStmt then_body;

  // Body of the branch where the condition is false.
  CompoundStmt else_body;
};

// Returns the type of `expr`.
inline std::string_view GetType(const Expr& expr) {
  return std::visit([](const auto& expr) { return expr.type; }, expr);
}

// Sets `type` as the type of `expr`.
inline void SetType(Expr& expr, std::string_view type) {
  std::visit([type](auto& expr) { expr.type = type; }, expr);
}

}  // namespace lucid
