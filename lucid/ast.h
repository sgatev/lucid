#pragma once

#include <string>
#include <variant>

namespace lucid {

// A statement that represents a function definition.
struct FuncDefStmt {
  // Name of the function.
  std::string name;
};

// A statement in the Lucid language.
using Stmt = std::variant<FuncDefStmt>;

}  // namespace lucid
