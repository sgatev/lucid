#pragma once

#include <ostream>
#include <string>
#include <utility>

#include "lucid/cfg.h"
#include "lucid/result.h"
#include "lucid/syntax/ast.h"

namespace lucid {

// An error that occurred while inferring static expressions.
class StaticError {
 public:
  explicit StaticError(std::string message) : message_(std::move(message)) {}

  bool operator==(const StaticError& other) const = default;

  friend std::ostream& operator<<(std::ostream& out, const StaticError error) {
    return out << error.message_;
  }

 private:
  std::string message_;
};

// Enhances expressions reachable from `stmt` with inferred static information.
//
// Returns an error if static constraints in `stmt` are not met.
//
// Requires:
// - `func_defs` must be associated with `ctx`.
// - `stmt` must be associated with `ctx`.
// - All functions called from `stmt` must be in `func_defs`.
Result<void, StaticError> InferStaticExprs(SyntaxContext& ctx,
                                           ControlFlowGraph& cfg);

}  // namespace lucid
