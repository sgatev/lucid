#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <utility>

#include "lucid/ast.h"

namespace lucid {

// An error that occurred while inferring types in Lucid code.
class TypeError {
 public:
  explicit TypeError(std::string message) : message_(std::move(message)) {}

  bool operator==(const TypeError& other) const = default;

  friend std::ostream& operator<<(std::ostream& out, const TypeError error) {
    return out << error.message_;
  }

 private:
  std::string message_;
};

// Enhances expressions reachable from `stmt` with inferred types.
//
// Returns an error if types in `stmt` are incompatible.
//
// Requires:
// - `func_defs` must be associated with `ctx`.
// - `stmt` must be associated with `ctx`.
// - All functions called from `stmt` must be in `func_defs`.
std::optional<TypeError> InferExprTypes(
    SyntaxContext& ctx, const std::vector<FuncDefStmt>& func_defs,
    FuncDefStmt& stmt);

}  // namespace lucid
