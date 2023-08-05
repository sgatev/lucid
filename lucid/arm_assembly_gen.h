#pragma once

#include <string>
#include <vector>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM assembly source code.
std::string GenerateArmStartSource();

// Generates 64-bit ARM assembly source code for `funcs`.
//
// All statements that are reachable from `funcs` must be allocated on `arena`.
std::string GenerateArmAssemblySource(const Arena<Stmt>& arena,
                                      const std::vector<FuncDefStmt>& funcs);

}  // namespace lucid
