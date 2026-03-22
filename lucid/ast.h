#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <variant>

#include "lucid/arena.h"
#include "lucid/string_index.h"
#include "lucid/successive_list.h"

namespace lucid {

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
struct FuncParam;

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
using StmtRef = Arena<Stmt>::Ref;

// A reference to an expression that can be dereferenced using an `Arena<Expr>`
// object.
using ExprRef = Arena<Expr>::Ref;

// A reference to a type that can be dereferenced using an `Arena<Type>` object.
using TypeRef = Arena<Type>::Ref;

using ParamRef = Arena<FuncParam>::Ref;

// A common base of all expressions.
struct ExprBase {
  // Type of the expression.
  TypeRef type;

  // Indicates whether the expression can be evaluated statically during
  // compilation.
  bool is_static;
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
  StringIndex::Ref value;
};

// An expression that represents a boolean literal.
struct BoolLitExpr : public ExprBase {
  // Value of the boolean.
  StringIndex::Ref value;
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

  bool operator==(const BasicType&) const = default;
};

// An array type in the Lucid language.
struct ArrayType {
  // Type of the elements of the array.
  TypeRef element_type_constraint;

  // Number of elements in the array.
  IntLitExpr size;
};

// Returns the type of `expr`.
inline TypeRef GetType(const Expr& expr) {
  return std::visit([](const auto& expr) { return expr.type; }, expr);
}

// Sets `type` as the type of `expr`.
inline void SetType(Expr& expr, TypeRef type) {
  std::visit([type](auto& expr) { expr.type = type; }, expr);
}

// A context for syntactic operations.
class SyntaxContext {
 public:
  // Adds `stmt` to the context.
  StmtRef Add(Stmt stmt) { return stmts_.Add(std::move(stmt)); }

  // Adds `expr` to the context.
  ExprRef Add(Expr expr) { return exprs_.Add(std::move(expr)); }

  // Adds `type` to the context.
  TypeRef Add(Type type) { return types_.Add(std::move(type)); }

  // Adds `param` to the context.
  ParamRef Add(FuncParam param) { return params_.Add(std::move(param)); }

  // Adds `ident` to the context.
  StringIndex::Ref AddIdent(std::string_view ident) {
    return idents_.ref(ident);
  }

  // Adds a unique ident to the context.
  StringIndex::Ref AddUniqueIdent() { return idents_.ref(); }

  // Creates an alias of `ref` in the context.
  StmtRef AliasStmt(StmtRef ref) { return stmts_.Alias(ref); }

  // Creates an alias of `ref` in the context.
  ExprRef AliasExpr(ExprRef ref) { return exprs_.Alias(ref); }

  // Creates an alias of `ref` in the context.
  ParamRef AliasParam(ParamRef ref) { return params_.Alias(ref); }

  // Returns the statement that `ref` refers to.
  Stmt& DerefStmt(StmtRef ref) { return stmts_.Get(ref); }
  const Stmt& DerefStmt(StmtRef ref) const { return stmts_.Get(ref); }

  // Returns the expression that `ref` refers to.
  Expr& DerefExpr(ExprRef ref) { return exprs_.Get(ref); }
  const Expr& DerefExpr(ExprRef ref) const { return exprs_.Get(ref); }

  // Returns the type that `ref` refers to.
  Type& DerefType(TypeRef ref) { return types_.Get(ref); }
  const Type& DerefType(TypeRef ref) const { return types_.Get(ref); }

  // Returns the parameter that `ref` refers to.
  FuncParam& DerefParam(ParamRef ref) { return params_.Get(ref); }
  const FuncParam& DerefParam(ParamRef ref) const { return params_.Get(ref); }

  // Returns the identifier that `ref` refers to.
  std::string_view DerefIdent(StringIndex::Ref ref) const {
    return idents_.deref(ref);
  }

  // Returns true if and only if `lhs` and `rhs` refer to equivalent statements.
  bool EquivStmts(StmtRef lhs, StmtRef rhs) const {
    return stmts_.Equiv(lhs, rhs);
  }

  std::size_t Size() const {
    return stmts_.Size() + exprs_.Size() + types_.Size();
  }

 private:
  Arena<Stmt> stmts_;
  Arena<Expr> exprs_;
  Arena<Type> types_;
  Arena<FuncParam> params_;
  StringIndex idents_;
};

}  // namespace lucid

namespace std {

template <>
struct hash<typename lucid::ExprRef> {
  size_t operator()(const lucid::ExprRef& ref) const {
    return hash<uint32_t>()(ref.id());
  }
};

template <>
struct hash<typename lucid::StmtRef> {
  size_t operator()(const lucid::StmtRef& ref) const {
    return hash<uint32_t>()(ref.id());
  }
};

template <>
struct hash<typename lucid::TypeRef> {
  size_t operator()(const lucid::TypeRef& ref) const {
    return hash<uint32_t>()(ref.id());
  }
};

template <>
struct hash<typename lucid::ParamRef> {
  size_t operator()(const lucid::ParamRef& ref) const {
    return hash<uint32_t>()(ref.id());
  }
};

}  // namespace std
