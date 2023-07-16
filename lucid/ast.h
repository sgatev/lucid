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

// An expression in the Lucid language.
using Expr = std::variant<IntLit>;

// A statement in the Lucid language.
using Stmt = std::variant<Expr, FuncDefStmt, ReturnStmt>;

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

}  // namespace lucid
