#pragma once

#include <expected>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "lucid/syntax/ast.h"

namespace lucid {

// An error that occurred while checking compilation requirements.
class CompError {
 public:
  explicit CompError(std::string message) : message_(std::move(message)) {}

  bool operator==(const CompError& other) const = default;

  friend std::ostream& operator<<(std::ostream& out, const CompError error) {
    return out << error.message_;
  }

 private:
  std::string message_;
};

// Returns an error if `stmt` does not meet compilation requirements.
//
// Requires:
// - `func_defs` must be associated with `syn_ctx`.
// - `stmt` must be associated with `syn_ctx`.
// - All functions called from `stmt` must be in `func_defs`.
std::expected<void, CompError> CheckComp(
    const SyntaxContext& syn_ctx, const std::vector<FuncDefStmt>& func_defs,
    const FuncDefStmt& stmt);

}  // namespace lucid
