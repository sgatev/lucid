#pragma once

#include <string>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

// Generates C++ source code for `stmt`.
//
// All statements that are reachable from `stmt` must be allocated on `arena`.
std::string GenerateCppSource(const Arena<Stmt>& arena, const Stmt& stmt);

}  // namespace lucid
