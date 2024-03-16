#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "lucid/arena.h"

namespace lucid {

struct CompoundStmt;
struct FuncDefStmt;
struct ReturnStmt;
struct DoStmt;
struct IntLitExpr;
struct BoolLitExpr;
struct StringLitExpr;
struct FuncCallExpr;
struct VarDeclStmt;
struct ArrayAssignStmt;
struct VarAssignStmt;
struct IdentExpr;
struct IndexExpr;
struct BinaryOpExpr;
struct IfStmt;
struct LoopStmt;
struct BasicType;
struct ArrayType;
struct BreakStmt;

// A type expression in the Lucid language.
using Type = std::variant<BasicType, ArrayType>;

// An expression in the Lucid language.
using Expr = std::variant<FuncCallExpr, IntLitExpr, BoolLitExpr, StringLitExpr,
                          IdentExpr, IndexExpr, BinaryOpExpr>;

// A statement in the Lucid language.
using Stmt =
    std::variant<VarDeclStmt, VarAssignStmt, ArrayAssignStmt, FuncDefStmt,
                 ReturnStmt, DoStmt, IfStmt, LoopStmt, BreakStmt>;

// A reference to a statement that can be dereferenced using an `Arena<Stmt>`
// object.
using StmtRef = ArenaRef<Stmt>;

// A reference to an expression that can be dereferenced using an `Arena<Expr>`
// object.
using ExprRef = ArenaRef<Expr>;

// A reference to a type that can be dereferenced using an `Arena<Type>` object.
using TypeRef = ArenaRef<Type>;

// A list of zero or more statements.
struct CompoundStmt {
  // Statements in the list.
  std::vector<StmtRef> statements;
};

// A function parameter.
struct FuncParam {
  // Type of the parameter.
  TypeRef type;

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
  TypeRef result_type;

  // Parameters of the function.
  std::vector<FuncParam> parameters;
};

// A statement that represents a return point in a function.
struct ReturnStmt {
  // Value that is returned by the function.
  ExprRef value;
};

// A statement that represents the execution of a procedure.
struct DoStmt {
  // The procedure that's being executed.
  // TODO: Find an appropriate representation for a procedure.
  ExprRef expr;
};

// An expression that represents an integer literal.
struct IntLitExpr {
  // Type of the expression.
  TypeRef type;

  // Value of the integer.
  std::string_view value;
};

// An expression that represents a boolean literal.
struct BoolLitExpr {
  // Type of the expression.
  TypeRef type;

  // Value of the boolean.
  std::string_view value;
};

// An expression that represents a string literal.
struct StringLitExpr {
  // Type of the expression.
  TypeRef type;

  // Value of the string.
  std::string_view value;
};

// An expression that represents a function call.
struct FuncCallExpr {
  // Type of the expression.
  TypeRef type;

  // Name of the function.
  std::string_view func_name;

  // Arguments to the function call.
  std::vector<ExprRef> arguments;
};

// A statement that represents a variable declaration.
struct VarDeclStmt {
  // Type of the variable.
  TypeRef type;

  // Name of the variable.
  std::string_view name;

  // Initializer expression.
  std::optional<ExprRef> init;
};

// A statement that represents assignment of an expression to a variable.
struct VarAssignStmt {
  // Name of the variable.
  std::string_view name;

  // Assigned expression.
  ExprRef expr;
};

// A statement that represents assignment of an expression to an array element.
struct ArrayAssignStmt {
  // Name of the array.
  std::string_view name;

  // Index in the array.
  ExprRef index;

  // Assigned expression.
  ExprRef expr;
};

// An expression that represents an identifier.
struct IdentExpr {
  // Type of the expression.
  TypeRef type;

  // Name of the identifier.
  std::string_view name;
};

// An expression that represents an indexing operation.
struct IndexExpr {
  // Type of the expression.
  TypeRef type;

  // Base of the indexing operation.
  ExprRef base;

  // Index of the indexing operation.
  ExprRef index;
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

  // Binary modulo operation.
  Mod,

  // Greater than comparison operation.
  Gt,

  // Less than comparison operation.
  Lt,

  // Equals comparison operator.
  Eq,

  // Not equals comparison operator.
  NotEq,
};

// An expression that represents a binary operation over the values of two
// sub-expressions.
struct BinaryOpExpr {
  // Type of the expression.
  TypeRef type;

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

// A statement that represents loop execution.
struct LoopStmt {
  // Body of the loop.
  CompoundStmt body;
};

// A statement that breaks from the inner-most loop execution.
struct BreakStmt {};

// Basic type in the Lucid language.
struct BasicType {
  // Name of the basic type.
  std::string_view name;

  bool operator==(const BasicType&) const = default;
};

// An array type in the Lucid language.
struct ArrayType {
  // Type of the elements of the array.
  TypeRef element_type;

  // Number of elements in the array.
  IntLitExpr size;
};

// Returns the type of `expr`.
inline TypeRef GetType(const Expr& expr) {
  return std::visit(
      [](const auto& expr) -> TypeRef {
        using T = std::decay_t<decltype(expr)>;
        if constexpr (std::is_same_v<T, Type>) {
          // TODO: Provide a TypeRef for meta type.
          return 0;
        } else {
          return expr.type;
        }
      },
      expr);
}

// Sets `type` as the type of `expr`.
inline void SetType(Expr& expr, TypeRef type) {
  std::visit(
      [type](auto& expr) {
        using T = std::decay_t<decltype(expr)>;
        if constexpr (!std::is_same_v<T, Type>) {
          expr.type = type;
        }
      },
      expr);
}

}  // namespace lucid
