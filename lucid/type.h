#pragma once

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

// Deduces the types of expressions in `stmt`.
//
// All statements that are reachable from `stmt` must be allocated on `arena`.
void DeduceTypes(Arena<Stmt>& arena, FuncDefStmt& stmt);

}  // namespace lucid
