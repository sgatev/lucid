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

  bool operator==(const TypeError& other) const = default;

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
//
// Requirements:
//  * Statements that are reachable from `func_defs` must be allocated on
//    `stmt_arena`.
//  * Expressions that are reachable from `func_defs` must be allocated on
//    `expr_arena`.
//  * Types that are reachable from `func_defs` must be allocated on
//    `type_arena`.
std::unordered_map<std::string_view, FuncType> ExtractFuncTypes(
    const Arena<Stmt>& stmt_arena, const Arena<Expr>& expr_arena,
    const Arena<Type>& type_arena, const std::vector<FuncDefStmt>& func_defs);

// Enhances expressions reachable from `stmt` with inferred types.
//
// Returns an error if types in `stmt` are incompatible.
//
// Requirements:
//  * Statements that are reachable from `stmt` must be allocated on
//    `stmt_arena`.
//  * Expressions that are reachable from `stmt` must be allocated on
//    `expr_arena`.
//  * Function calls that are reachable from `stmt` must refer to functions
//    in `func_types`.
std::optional<TypeError> InferExprTypes(
    const Arena<Stmt>& stmt_arena, Arena<Expr>& expr_arena,
    Arena<Type>& type_arena,
    const std::unordered_map<std::string_view, FuncType>& func_types,
    FuncDefStmt& stmt);

}  // namespace lucid
