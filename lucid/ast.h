#pragma once

#include <string>
#include <variant>

#include "lucid/arena.h"

namespace lucid {

struct CompoundStmt;
struct FuncDefStmt;
struct ReturnStmt;

// A statement in the Lucid language.
using Stmt = std::variant<FuncDefStmt, ReturnStmt>;

// A reference to a statement that can be dereferenced using an `Arena<Stmt>`
// object.
using StmtRef = ArenaRef<Stmt>;

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
struct ReturnStmt {};

}  // namespace lucid
