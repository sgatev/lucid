#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "lucid/arena.h"

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

// A list of references.
template <typename T>
class List {
 public:
  class iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using element_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

    iterator() : expr_(0) {}

    explicit iterator(T expr) : expr_(expr) {}

    iterator(const iterator& other) = default;
    iterator(iterator&& other) = default;
    iterator& operator=(const iterator& other) = default;
    iterator& operator=(iterator&& other) = default;

    iterator& operator++() {
      ++expr_;
      return *this;
    }

    iterator operator++(int) {
      ++expr_;
      return *this;
    }

    iterator& operator--() {
      --expr_;
      return *this;
    }

    iterator operator--(int) {
      --expr_;
      return *this;
    }

    bool operator==(const iterator& other) const {
      return expr_ == other.expr_;
    }

    bool operator!=(const iterator& other) const { return !(*this == other); }

    const T& operator*() const { return expr_; }
    pointer operator->() { return &expr_; }

   private:
    T expr_;
  };

  List() : size_(0), first_(0) {}

  List(std::uint32_t size, T first) : size_(size), first_(first) {}

  // Returns the reference at the given index.
  T operator[](std::uint8_t i) const { return first_ + i; }

  // Returns the size of the list.
  std::uint32_t size() const { return size_; }

  // Returns an iterator to the first reference of the list.
  auto begin() const { return iterator(first_); }

  // Returns an iterator to the value following the last reference of the list.
  auto end() const { return iterator(first_ + size_); }

 private:
  std::uint32_t size_;
  T first_;
};

static_assert(std::bidirectional_iterator<List<StmtRef>::iterator>);

// A function parameter.
struct FuncParam {
  // Name of the parameter.
  std::string_view name;

  // Type of the parameter.
  TypeRef type;

  bool operator==(const FuncParam&) const = default;
};

// A statement that represents a function definition.
struct FuncDefStmt {
  // Name of the function.
  std::string_view name;

  // Parameters of the function.
  std::vector<FuncParam> parameters;

  // Type of the result of the function.
  TypeRef result_type;

  // Body of the function.
  List<StmtRef> stmts;
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
  List<ExprRef> args;
};

// A statement that represents a variable declaration.
struct VarDeclStmt {
  // Name of the variable.
  std::string_view name;

  // Type of the variable.
  TypeRef type;

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
  List<StmtRef> then_stmts;

  // Body of the branch where the condition is false.
  List<StmtRef> else_stmts;
};

// A statement that represents loop execution.
struct LoopStmt {
  // Body of the loop.
  List<StmtRef> stmts;
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
  StmtRef Add(Stmt stmt) { return stmts_.add(std::move(stmt)); }

  // Adds `expr` to the context.
  ExprRef Add(Expr expr) { return exprs_.add(std::move(expr)); }

  // Adds `type` to the context.
  TypeRef Add(Type type) { return types_.add(std::move(type)); }

  // Creates an alias of `ref` in the context.
  StmtRef AliasStmt(StmtRef ref) { return stmts_.alias(ref); }

  // Creates an alias of `ref` in the context.
  ExprRef AliasExpr(ExprRef ref) { return exprs_.alias(ref); }

  // Returns the statement that `ref` refers to.
  Stmt& DerefStmt(StmtRef ref) { return stmts_.get(ref); }
  const Stmt& DerefStmt(StmtRef ref) const { return stmts_.get(ref); }

  // Returns the expression that `ref` refers to.
  Expr& DerefExpr(ExprRef ref) { return exprs_.get(ref); }
  const Expr& DerefExpr(ExprRef ref) const { return exprs_.get(ref); }

  // Returns the type that `ref` refers to.
  Type& DerefType(TypeRef ref) { return types_.get(ref); }
  const Type& DerefType(TypeRef ref) const { return types_.get(ref); }

  // Returns true if and only if `lhs` and `rhs` refer to equivalent statements.
  bool EquivStmts(StmtRef lhs, StmtRef rhs) const {
    return stmts_.equiv(lhs, rhs);
  }

  std::size_t Size() const {
    return stmts_.size() + exprs_.size() + types_.size();
  }

 private:
  Arena<Stmt> stmts_;
  Arena<Expr> exprs_;
  Arena<Type> types_;
};

}  // namespace lucid
