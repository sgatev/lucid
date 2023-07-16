#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "lucid/arena.h"

namespace lucid {

struct CompoundStmt;
struct FuncDefStmt;
struct ReturnStmt;
struct IntLit;
struct FuncCallExpr;
struct VarDeclStmt;

// An expression in the Lucid language.
using Expr = std::variant<FuncCallExpr, IntLit>;

// A statement in the Lucid language.
using Stmt = std::variant<Expr, VarDeclStmt, FuncDefStmt, ReturnStmt>;

// A reference to a statement that can be dereferenced using an `Arena<Stmt>`
// object.
using StmtRef = ArenaRef<Stmt>;

// A reference to an expression that can be dereferenced using an `Arena<Stmt>`
// object.
using ExprRef = StmtRef;

// A collection of zero or more statements.
struct CompoundStmt {
  std::vector<StmtRef> statements;
};

// A statement that represents a function definition.
struct FuncDefStmt {
  // Name of the function.
  std::string name;

  // Body of the function.
  CompoundStmt body;

  // Type of the result of the function.
  std::string result_type;
};

// A statement that represents a return point in a function.
struct ReturnStmt {
  // Value that is returned by the function.
  ExprRef value;
};

// An integer literal.
struct IntLit {
  // Value of the integer.
  int32_t value;
};

// An expression that represents a function call.
struct FuncCallExpr {
  // Name of the function.
  std::string func_name;
};

// A statement that represents a variable declaration.
struct VarDeclStmt {
  // Type of the variable.
  std::string type;

  // Name of the variable.
  std::string name;

  // Initializer expression.
  ExprRef init;
};

}  // namespace lucid
