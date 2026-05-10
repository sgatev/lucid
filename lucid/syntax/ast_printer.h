#pragma once

#include <ostream>

#include "lucid/syntax/ast.h"

namespace lucid {

// Prints a text representation of the abstract syntax tree statement `stmt` to
// `out`.
//
// Requires:
// - `stmt` must be constructed in `ctx`.
void Print(const SyntaxContext ctx, const FuncDefStmt& stmt, std::ostream& out);

// Prints a text representation of the abstract syntax tree statement
// referenced by `ref` to `out`.
//
// Requires:
// - `ref` must be constructed in `ctx`.
void PrintStmt(const SyntaxContext ctx, StmtRef ref, std::ostream& out);

// Prints a text representation of the abstract syntax tree expression
// referenced by `ref` to `out`.
//
// Requires:
// - `ref` must be constructed in `ctx`.
void PrintExpr(const SyntaxContext ctx, ExprRef ref, std::ostream& out);

}  // namespace lucid
