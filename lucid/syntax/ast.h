#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

#include "lucid/core/container/arena.h"
#include "lucid/core/container/successive_list.h"
#include "lucid/core/string/index.h"

namespace lucid {

struct FuncDefStmt;
struct TypeDefStmt;
struct ReturnStmt;
struct DoStmt;
struct IntLitExpr;
struct BoolLitExpr;
struct StringLitExpr;
struct FuncCallExpr;
struct VarDeclStmt;
struct ArrayAssignStmt;
struct FieldAssignStmt;
struct VarAssignStmt;
struct IdentExpr;
struct IndexExpr;
struct FieldAccessExpr;
struct BinaryOpExpr;
struct IfStmt;
struct LoopStmt;
struct BasicType;
struct ArrayType;
struct TupleType;
struct BreakStmt;
struct FuncParam;

// A type expression in the Lucid language.
using Type = std::variant<BasicType, ArrayType, TupleType>;

// An expression in the Lucid language.
using Expr = std::variant<FuncCallExpr, IntLitExpr, BoolLitExpr, StringLitExpr,
                          IdentExpr, IndexExpr, FieldAccessExpr, BinaryOpExpr>;

// A statement in the Lucid language.
using Stmt = std::variant<VarDeclStmt, VarAssignStmt, ArrayAssignStmt,
                          FieldAssignStmt, FuncDefStmt, TypeDefStmt, ReturnStmt,
                          DoStmt, IfStmt, LoopStmt, BreakStmt>;

// A definition in the Lucid language.
using Def = std::variant<FuncDefStmt, TypeDefStmt>;

// A reference to a statement that can be dereferenced using an `Arena<Stmt>`
// object.
using StmtRef = Arena<Stmt>::Ref;

inline std::size_t Hash(const lucid::StmtRef& ref) { return Hash(ref.id()); }

// A reference to an expression that can be dereferenced using an `Arena<Expr>`
// object.
using ExprRef = Arena<Expr>::Ref;

inline std::size_t Hash(const lucid::ExprRef& ref) { return Hash(ref.id()); }

// A reference to a type that can be dereferenced using an `Arena<Type>` object.
using TypeRef = Arena<Type>::Ref;

inline std::size_t Hash(const lucid::TypeRef& ref) { return Hash(ref.id()); }

// A reference to a function parameter that can be dereferenced using an
// `Arena<FuncParam>` object.
using ParamRef = Arena<FuncParam>::Ref;

inline std::size_t Hash(const lucid::ParamRef& ref) { return Hash(ref.id()); }

// A common base of all expressions.
struct ExprBase {
  // Type of the expression.
  TypeRef type;

  // Whether the expression should be evaluated during compilation.
  bool is_comp : 1 = false;
};

// A function parameter.
struct FuncParam {
  // Name of the parameter.
  StringIndex::Ref name;

  // Type of the parameter.
  TypeRef type_constraint;
};

// A statement that represents a function definition.
struct FuncDefStmt {
  // Name of the function.
  StringIndex::Ref name;

  // Parameters of the function.
  SuccessiveList<ParamRef> params;

  // Type of the result of the function.
  TypeRef result_type;

  // Body of the function.
  SuccessiveList<StmtRef> stmts;

  // Whether the function can be evaluated during compilation.
  bool is_comp = false;
};

// A statement that represents a type definition.
struct TypeDefStmt {
  // Name of the type.
  StringIndex::Ref name;

  // Definition of the type.
  TypeRef type;
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
struct IntLitExpr : public ExprBase {
  // Value of the integer.
  std::int64_t value;
};

// An expression that represents a boolean literal.
struct BoolLitExpr : public ExprBase {
  // Value of the boolean.
  bool value;
};

// An expression that represents a string literal.
struct StringLitExpr : public ExprBase {
  // Value of the string.
  StringIndex::Ref value;
};

// An expression that represents a function call.
struct FuncCallExpr : public ExprBase {
  // Name of the function.
  StringIndex::Ref func_name;

  // Arguments to the function call.
  SuccessiveList<ExprRef> args;
};

// A statement that represents a variable declaration.
struct VarDeclStmt {
  // Name of the variable.
  StringIndex::Ref name;

  // Type of the variable.
  TypeRef type_constraint;

  // Initializer expression.
  std::optional<ExprRef> init;

  // Whether the variable is initialized during compilation.
  bool is_comp = false;
};

// A statement that represents assignment of an expression to a variable.
struct VarAssignStmt {
  // Name of the variable.
  StringIndex::Ref name;

  // Assigned expression.
  ExprRef expr;
};

// A statement that represents assignment of an expression to an array element.
struct ArrayAssignStmt {
  // Name of the array.
  StringIndex::Ref name;

  // Index in the array.
  ExprRef index;

  // Assigned expression.
  ExprRef expr;
};

// A statement that represents assignment of an expression to a field.
struct FieldAssignStmt {
  // Base of the field.
  ExprRef base;

  // Name of the field.
  StringIndex::Ref field_name;

  // Assigned expression.
  ExprRef expr;
};

// An expression that represents an identifier.
struct IdentExpr : public ExprBase {
  // Name of the identifier.
  StringIndex::Ref name;
};

// An expression that represents an indexing operation.
struct IndexExpr : public ExprBase {
  // Base of the indexing operation.
  ExprRef base;

  // Index of the indexing operation.
  ExprRef index;
};

// An expression that represents a field access operation.
struct FieldAccessExpr : public ExprBase {
  // Base of the field.
  ExprRef base;

  // Name of the field.
  StringIndex::Ref field_name;
};

// A binary operation kind.
enum class BinaryOp : std::uint8_t {
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

inline std::string to_string(BinaryOp op) {
  switch (op) {
    case BinaryOp::Add:
      return "Add";
    case BinaryOp::Sub:
      return "Sub";
    case BinaryOp::Mul:
      return "Mul";
    case BinaryOp::Div:
      return "Div";
    case BinaryOp::Mod:
      return "Mod";
    case BinaryOp::Gt:
      return "Gt";
    case BinaryOp::Lt:
      return "Lt";
    case BinaryOp::Eq:
      return "Eq";
    case BinaryOp::NotEq:
      return "NotEq";
  }
}

// An expression that represents a binary operation over the values of two
// sub-expressions.
struct BinaryOpExpr : public ExprBase {
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
  SuccessiveList<StmtRef> then_stmts;

  // Body of the branch where the condition is false.
  SuccessiveList<StmtRef> else_stmts;
};

// A statement that represents loop execution.
struct LoopStmt {
  // Body of the loop.
  SuccessiveList<StmtRef> stmts;
};

// A statement that breaks from the inner-most loop execution.
struct BreakStmt {};

// Basic type in the Lucid language.
struct BasicType {
  // Name of the basic type.
  StringIndex::Ref name;

  // Size of the basic type in memory.
  std::size_t size;

  bool operator==(const BasicType&) const = default;
};

// An array type in the Lucid language.
struct ArrayType {
  // Type of the elements of the array.
  TypeRef element_type_constraint;

  // Number of elements in the array.
  IntLitExpr size;
};

// A tuple type in the Lucid language.
struct TupleType {
  // Fields of the tuple.
  SuccessiveList<ParamRef> fields;

  bool operator==(const TupleType&) const = default;
};

// Returns the type of `expr`.
inline TypeRef GetType(const Expr& expr) {
  return std::visit([](const auto& expr) { return expr.type; }, expr);
}

// Sets `type` as the type of `expr`.
inline void SetType(Expr& expr, TypeRef type) {
  std::visit([type](auto& expr) { expr.type = type; }, expr);
}

}  // namespace lucid
