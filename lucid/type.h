#pragma once

#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
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

// A function type.
struct FuncType {
  // Type of the result of the function.
  std::string_view result_type;

  // Types of parameters of the function.
  std::span<const FuncParam> parameters;

  bool operator==(const FuncType& func_type) const {
    if (result_type != func_type.result_type) return false;
    if (parameters.size() != func_type.parameters.size()) return false;
    for (int i = 0; i < parameters.size(); ++i) {
      if (parameters[i] != func_type.parameters[i]) return false;
    }
    return true;
  }
};

// Creates a map from names of functions to their respective types.
std::unordered_map<std::string_view, FuncType> ExtractFuncTypes(
    const std::vector<FuncDefStmt>& func_defs);

// Enhances expressions reachable from `stmt` with inferred types.
//
// Returns an error if types in `stmt` are incompatible.
//
// Requirements:
//  * All statements that are reachable from `stmt` must be allocated on
//    `arena`.
//  * All function calls that are reachable from `stmt` must refer to functions
//    in `func_types`.
std::optional<TypeError> InferExprTypes(
    Arena<Stmt>& arena,
    const std::unordered_map<std::string_view, FuncType>& func_types,
    FuncDefStmt& stmt);

}  // namespace lucid
