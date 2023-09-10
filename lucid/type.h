#pragma once

#include <optional>
#include <string>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

// Deduces the types of expressions in `stmt`.
//
// Returns an error if types in `stmt` are incompatible.
//
// All statements that are reachable from `stmt` must be allocated on `arena`.
std::optional<std::string> DeduceTypes(Arena<Stmt>& arena, FuncDefStmt& stmt);

}  // namespace lucid
