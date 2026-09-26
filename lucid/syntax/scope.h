#pragma once

#include <expected>
#include <ostream>
#include <string>
#include <utility>

#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {

// An error that occurred while working out what the names in Lucid code
// stand for.
class ScopeError {
 public:
  explicit ScopeError(std::string message) : message_(std::move(message)) {}

  bool operator==(const ScopeError& other) const = default;

  friend std::ostream& operator<<(std::ostream& out, const ScopeError& error) {
    return out << error.message_;
  }

 private:
  std::string message_;
};

// Gives every variable declared in `stmt` a name of its own, and rewrites
// every use of a variable to the name of the declaration standing over it.
//
// A declaration holds from where it is made to the end of the block holding
// it, so one made inside a block is gone after it and one that repeats a name
// stands over the earlier declaration only while its own block lasts. Every
// pass after this one tells two variables apart by their names alone, which
// is what a name of its own for each declaration leaves them able to do.
//
// Returns an error where a name is used with no declaration standing over it.
//
// Requires:
// - `stmt` must be associated with `syn_ctx`.
std::expected<void, ScopeError> ResolveNames(SyntaxContext& syn_ctx,
                                             FuncDefStmt& stmt);

}  // namespace lucid
