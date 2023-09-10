#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <utility>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

// An error that occurred while inferring types in Lucid code.
class TypeError {
 public:
  explicit TypeError(std::string message) : message_(std::move(message)) {}

  bool operator==(const TypeError& other) const {
    return message_ == other.message_;
  }

  friend std::ostream& operator<<(std::ostream& out, const TypeError error) {
    return out << error.message_;
  }

 private:
  std::string message_;
};

// Infers the types of expressions in `stmt`.
//
// Returns an error if types in `stmt` are incompatible.
//
// All statements that are reachable from `stmt` must be allocated on `arena`.
std::optional<TypeError> InferExpressionTypes(Arena<Stmt>& arena,
                                              FuncDefStmt& stmt);

}  // namespace lucid
