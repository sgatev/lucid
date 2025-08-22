#pragma once

#include "lucid/ast.h"

namespace lucid {

// Prints AST nodes reachable from `stmt`.
//
// Requires:
// - `stmt` must be associated with `ctx`.
void Print(const SyntaxContext ctx, const FuncDefStmt& stmt);

}  // namespace lucid
