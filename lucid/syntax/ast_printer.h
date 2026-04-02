#pragma once

#include "lucid/syntax/ast.h"

namespace lucid {

// Prints AST nodes reachable from `stmt`.
//
// Requires:
// - `stmt` must be associated with `ctx`.
void Print(const SyntaxContext ctx, const FuncDefStmt& stmt);

// Prints the statement node referenced by `ref`.
//
// Requires:
// - `ref` must be associated with `ctx`.
void PrintStmt(const SyntaxContext ctx, StmtRef ref);

// Prints the statement node referenced by `ref`.
//
// Requires:
// - `ref` must be associated with `ctx`.
void PrintExpr(const SyntaxContext ctx, ExprRef ref);

}  // namespace lucid
