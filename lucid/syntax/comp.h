#pragma once

#include <expected>
#include <ostream>
#include <string>
#include <utility>

#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

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

// Marks sub-expressions of `stmt` that must be evaluated during compilation.
// Returns an error if `stmt` does not meet compilation requirements.
//
// Requires:
// - `stmt` must be associated with `syn_ctx`.
// - All functions called from `stmt` must be in `func_defs`.
std::expected<void, CompError> CheckComp(SyntaxContext& syn_ctx,
                                         FuncDefStmt& stmt);

}  // namespace lucid
